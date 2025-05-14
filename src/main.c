#include "config.h"
#include "construct.h"
#include "sample.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <linux/filter.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <inttypes.h>


void Scan(uint64_t step) {
    uint8_t buffer[1500];
    struct ethhdr eth;
    struct ip6_hdr ip6;
    struct icmp6_hdr icmp6;
    int index = prefix_table_size - 1;

    for (uint64_t i = 0; i < step; ++i) {
        // If there are prefixes with remaining budget, randomly select one
        if (nonzero_budget_count) {
            uint64_t nonzero_index = rand() % nonzero_budget_count;
            index = nonzero_budget_indices[nonzero_index];
            
            // Decrease the allocated budget for the selected prefix
            prefix_allocated_budget[index]--;
            
            // If the budget reaches zero, remove the prefix from the nonzero list
            if (prefix_allocated_budget[index] == 0) {
                nonzero_budget_indices[nonzero_index] = nonzero_budget_indices[--nonzero_budget_count];
            }
        }
        // If no budgeted prefixes remain, select a random prefix from the table
        else {
            index = rand() % prefix_table_size;
        }

        // Construct an ICMPv6 packet
        constructICMPv6Packet(&eth, &ip6, &icmp6, index);

        // Create a buffer large enough to store the entire packet
        unsigned char buffer[sizeof(struct ethhdr) + sizeof(struct ip6_hdr) + sizeof(struct icmp6_hdr)];

        // Copy Ethernet, IPv6, and ICMPv6 headers into the buffer sequentially
        memcpy(buffer, &eth, sizeof(struct ethhdr));
        memcpy(buffer + sizeof(struct ethhdr), &ip6, sizeof(struct ip6_hdr));
        memcpy(buffer + sizeof(struct ethhdr) + sizeof(struct ip6_hdr), &icmp6, sizeof(struct icmp6_hdr));

        // Send the constructed packet via raw socket
        send(fd, buffer, sizeof(buffer), 0);

        // Increment the sent packet count for the selected prefix
        prefix_table[index].sent_packets++;
    }
}

void* Recv(void* arg) {
    uint8_t buffer[1000];
    ssize_t received_bytes;
    struct sockaddr_storage src_addr;
    socklen_t addr_len = sizeof(src_addr);

    // Open the output file in write mode (overwrites existing content)
    FILE* output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        perror("Failed to create file");
        exit(1);
    }

    while (1) {
        // Receive a packet
        received_bytes = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&src_addr, &addr_len);
        if (received_bytes < 0) {
            perror("recvfrom");
            continue;
        }

        // Extract Ethernet header
        struct ether_header* eth_hdr = (struct ether_header*)buffer;
        if (ntohs(eth_hdr->ether_type) != ETH_P_IPV6) {
            continue;  // Ignore non-IPv6 packets
        }

        // Extract IPv6 header
        struct ip6_hdr* ipv6_hdr = (struct ip6_hdr*)(buffer + sizeof(struct ether_header));
        if (ipv6_hdr->ip6_nxt != IPPROTO_ICMPV6) {
            continue;  // Ignore non-ICMPv6 packets
        }

        // Extract ICMPv6 header
        struct icmp6_hdr* icmp6_hdr = (struct icmp6_hdr*)(buffer + sizeof(struct ether_header) + sizeof(struct ip6_hdr));
        if (icmp6_hdr->icmp6_type < 1 || icmp6_hdr->icmp6_type > 4) {
            continue;  // Only process ICMPv6 error messages (types 1-4)
        }

        uint8_t icmp_type = buffer[54];
        uint8_t icmp_code = buffer[55];
        uint32_t status_code;

        // Process ICMP Destination Unreachable (Type 1, Code 0) or Time Exceeded (Type 3, Code 0)
        if ((icmp_type == 1 && icmp_code == 0) || (icmp_type == 3 && icmp_code == 0)) {
            status_code = ntohl(*(uint32_t*)&buffer[106]);

            struct in6_addr target_ip;
            memcpy(&target_ip, buffer + 86, sizeof(struct in6_addr));

            char target_ip_str[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &target_ip, target_ip_str, sizeof(target_ip_str));

            struct in6_addr router_ip;
            memcpy(&router_ip, buffer + 22, sizeof(struct in6_addr));

            char router_ip_str[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &router_ip, router_ip_str, sizeof(router_ip_str));

            // Extract last 4 bytes of the destination IP address and validate checksum
            uint32_t last_4_bytes;
            memcpy(&last_4_bytes, target_ip.s6_addr + 12, sizeof(uint32_t));
            last_4_bytes = ntohl(last_4_bytes);

            uint32_t computed_checksum = murmur3(target_ip.s6_addr, 12, 0x11112222);
            if (last_4_bytes != computed_checksum) continue;

            uint64_t prefix_index = status_code >> 8;
            if (prefix_index >= prefix_table_size) continue;

            // Verify the router IP address is different from the destination IP address
            uint32_t checksum_target = murmur3(target_ip.s6_addr, 12, 0x11112222);
            uint32_t checksum_router = murmur3(router_ip.s6_addr, 12, 0x11112222);
            if (checksum_target == checksum_router) continue;

            // Apply Bloom filter to avoid duplicate entries
            uint32_t bloom_index_1 = murmur3(buffer + 22, 16, 0x12345678);
            uint32_t bloom_index_2 = murmur3(buffer + 22, 16, 0x87654321);
            if ((bloom_filter[bloom_index_1 / 8] & (1 << (bloom_index_1 % 8))) &&
                (bloom_filter[bloom_index_2 / 8] & (1 << (bloom_index_2 % 8)))) {
                continue;
            }

            // Update Bloom filter
            bloom_filter[bloom_index_1 / 8] |= (1 << (bloom_index_1 % 8));
            bloom_filter[bloom_index_2 / 8] |= (1 << (bloom_index_2 % 8));

            // Increase packet reception count for the prefix
            __sync_add_and_fetch(&prefix_table[prefix_index].received_packets, 1);

            uint8_t hop_limit = status_code & 0xFF;

            // Write the discovered router interface information to file
            fprintf(output_file, "%lu %d %s %s\n", prefix_index, hop_limit, router_ip_str, target_ip_str);
            total_hits++;

            // Update observations based on the hop limit and prefix index
            update_observations(hop_limit, prefix_index);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 7) {
        printf("Usage: %s <source_mac> <source_ip> <gateway_mac> <input_filename> <output_filename>\n", argv[0]);
        return 1;
    }

    // Assign values from the command line arguments
    interface_name = argv[1];
    source_mac = argv[2];
    source_ip = argv[3];
    gateway_mac = argv[4];
    input_filename = argv[5]; 
    output_filename = argv[6];

    struct ifreq ifr;
    struct sockaddr_ll sll;

    // Create a raw socket for packet transmission
    fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IPV6));
    if (fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Configure network interface
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);

    // Retrieve the interface index using ioctl
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl(SIOCGIFINDEX)");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the specified network interface
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_IPV6);
    sll.sll_ifindex = ifr.ifr_ifindex; 

    if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        perror("bind");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the bloom filter
    bloom_filter = calloc(1, bloom_filter_SIZE);
    if (!bloom_filter) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    processAndParseCIDR();
    printf("prefix_table_size: %ld\n", prefix_table_size);

    // Start the receiving thread
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, Recv, NULL);

    uint64_t step_size = 1e6;
    uint64_t total_sent = 0;
    uint64_t budget_limit = 1e8;

    while (total_sent < budget_limit) {
        step_size = is_first_round ? 1e7 : 1e6;
        uint64_t allocated_packets = 0;
        double total_weight = 0;
        nonzero_budget_count = 0;

        // Compute weights for each prefix based on success ratio and exploration factor
        for (int index = 0; index < prefix_table_size; index++) {
            double success_ratio = (prefix_table[index].received_packets / log(prefix_table[index].sent_packets + 10));
            double exploitation = success_ratio;
            double exploration = 100 * (1 / (prefix_table[index].sent_packets + 1)); // UCB exploration factor
            prefix_table[index].weight = exploitation + exploration;
            total_weight += prefix_table[index].weight;
        }


        // Allocate probing budget based on calculated weights
        for (int index = 0; index < prefix_table_size; index++) {
            if (!is_first_round) {
                prefix_allocated_budget[index] = (uint64_t)((double)(step_size * prefix_table[index].weight) / total_weight);
            } else {
                prefix_allocated_budget[index] = (uint64_t)(step_size * 1.0 / prefix_table_size);
            }
            if (prefix_allocated_budget[index]) {
                nonzero_budget_indices[nonzero_budget_count++] = index;
            }
            allocated_packets += prefix_allocated_budget[index];
        }
     
       // Execute probing with allocated packets
        if (budget_limit - total_sent < allocated_packets) {
            Scan(budget_limit - total_sent);
            total_sent += budget_limit - total_sent;
        } else {
            Scan(allocated_packets);
            total_sent += allocated_packets;
        }

        printf("%" PRIu64 " packets sent, %" PRIu64 " responses received, success rate: %lf\n", 
            total_sent, total_hits, 1.0 * total_hits / total_sent);

        is_first_round = 0;
    }

    sleep(10);
    close(fd);
    free(bloom_filter);
    return 0;
}
