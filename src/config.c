#include "config.h"

uint8_t *bloom_filter = NULL;
int fd = 0;
PrefixInfo prefix_table[MAX_PREFIX_TABLE_SIZE];
size_t prefix_table_size = 0;
uint64_t total_hits = 0;
int is_first_round = 1;
uint64_t prefix_allocated_budget[MAX_PREFIX_TABLE_SIZE];
uint64_t nonzero_budget_indices[MAX_PREFIX_TABLE_SIZE];
size_t nonzero_budget_count;
char *interface_name;
char *source_mac;
char *source_ip;
char *gateway_mac;
char* input_filename;
char* output_filename;