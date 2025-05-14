#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define bloom_filter_SIZE (1 << 30)
#define MAX_PREFIX_TABLE_SIZE (1 << 24)

#ifndef htonll
#define htonll(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

typedef struct {
    uint64_t prefix_stub;
    uint64_t mask_suffix;
    uint64_t received_packets;
    uint64_t sent_packets;
    uint64_t prefix_length;

    double weight;
    double alpha_param[255];
    double beta_param[255];

    uint8_t hop_candidates[255];
    uint8_t hop_candidate_flags[255];
    uint8_t hop_candidate_count;
} PrefixInfo;


extern uint8_t *bloom_filter;
extern int fd;
extern PrefixInfo prefix_table[MAX_PREFIX_TABLE_SIZE];
extern size_t prefix_table_size;
extern uint64_t total_hits;
extern int is_first_round;
extern uint64_t prefix_allocated_budget[MAX_PREFIX_TABLE_SIZE];
extern uint64_t nonzero_budget_indices[MAX_PREFIX_TABLE_SIZE];
extern size_t nonzero_budget_count;
extern char *interface_name;
extern char *source_mac;
extern char *source_ip;
extern char *gateway_mac;
extern char* input_filename;
extern char* output_filename;
#endif
