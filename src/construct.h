#ifndef CONSTRUCT_H
#define CONSTRUCT_H

#include "config.h"
#include "hash.h"

#include <stdint.h>
#include <netinet/ip6.h>
#include <netinet/if_ether.h>
#include <netinet/icmp6.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>


uint32_t generate_32bit_random();
void constructICMPv6Packet(struct ethhdr *eth_hdr, struct ip6_hdr *ip6_hdr, struct icmp6_hdr *icmp6_hdr, int index);

#endif
