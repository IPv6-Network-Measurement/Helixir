#include "config.h"

// Function to process and parse IPv6 CIDR prefixes from a file
void processAndParseCIDR() {
    FILE* file = fopen(input_filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char line[100]; // Buffer for each line
    size_t lineCount = 0;

    while (fgets(line, sizeof(line), file)) {
        // Remove the newline character
        char* newline = strchr(line, '\n');
        if (newline) {
            *newline = '\0';
        }

        // Parse CIDR format
        struct in6_addr ip6;
        char* slashPos = strchr(line, '/');
        if (!slashPos) {
            fprintf(stderr, "Invalid CIDR format: %s\n", line);
            continue;
        }

        // Extract the IP address
        char ipStr[40];
        strncpy(ipStr, line, slashPos - line);
        ipStr[slashPos - line] = '\0';

        // Convert prefix length to an integer
        int prefixLen = atoi(slashPos + 1);
        if (prefixLen < 0 || prefixLen > 128) {
            fprintf(stderr, "Invalid prefix length: %s\n", line);
            continue;
        }

        // Convert IP address string to binary format
        if (inet_pton(AF_INET6, ipStr, &ip6) != 1) {
            fprintf(stderr, "Invalid IPv6 address: %s\n", line);
            continue;
        }

        // Extract the first 64 bits of the IPv6 address as the prefix stub
        uint64_t stub = htonll(*(uint64_t*)(&ip6.s6_addr[0]));

        // Generate a subnet mask based on the prefix length
        uint64_t mask;
        if (prefixLen <= 64) {
            mask = (~0ULL >> prefixLen) & 0xFFFFFFFFFFFFFFFF; // Convert prefix length to a lower 64-bit mask
        } 

        // Store the parsed prefix in the prefix table
        prefix_table[prefix_table_size].prefix_stub = stub;
        prefix_table[prefix_table_size].mask_suffix = mask;
        prefix_table[prefix_table_size].prefix_length = prefixLen;
        prefix_table[prefix_table_size].sent_packets = 0;
        prefix_table[prefix_table_size].received_packets = 0;

        // Increase the prefix table size
        prefix_table_size++;
        lineCount++;

        // Stop processing if the limit is reached
        if (lineCount == (MAX_PREFIX_TABLE_SIZE)) {
            fprintf(stderr, "Reached the maximum limit of lines to process.\n");
            break;
        }
    }

    fclose(file);
}
