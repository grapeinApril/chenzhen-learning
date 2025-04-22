#ifndef RAFT_HASH_H
#define RAFT_HASH_H
#include "raft_comon.h"

typedef struct hash_node_s {
    raft_node_t *key;
    raft_group_member_t *value;
    struct hash_node_t *next;
} raft_node_t;

typedef struct raft_hash_s {
    raft_node_t **table;
    int size;
} raft_hash_t;

raft_hash_t *raft_init_hash_table(int size);
unsigned int raf_hash(const raft_node_t *node, int size);
void raft_hash_insert(raft_hash_t *ht, raft_node_t *key, raft_group_member_t *value);
raft_group_member_t *raft_hash_find(raft_hash_t *ht, raft_node_t *key);
void raft_hash_delete(raft_hash_t *ht, raft_node_t *key);
void raft_free_hash_table(raft_hash_t *ht);
#endif