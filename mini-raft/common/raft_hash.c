/**
 * raft_hash.c
 * @author chenzhen
 * 
 */

 #include "raft_hash.h"
 #include "raft_utils.h"

 raft_hash_t *raft_init_hash_table(int size) {
    raft_hash_t *ht = (raft_hash_t *)raft_malloc(sizeof(raft_hash_t));
    if (ht == NULL) {
        return NULL;
    }
    
    ht->table = (raft_node_t **)raft_malloc(size * sizeof(raft_node_t *));
    ht->size = size;
    return ht;
 }

unsigned int raf_hash(const raft_node_t *node, int size) {
    unsigned int hash = 0;
    for (int i = 0; node->value[i] != '\0'; i++) {
        hash = node->value[i] + (hash << 6) + (hash << 16) - hash;
    }
    return hash % size;
}

void raft_hash_insert(raft_hash_t *ht, raft_node_t *key, raft_group_member_t *value) {
    unsigned int index = raf_hash(key->value, ht->size);
    raft_hash_node_t *new_node = (raft_hash_node_t *)raft_malloc(sizeof(raft_hash_node_t));
    new_node->key = key;
    new_node->value = value;
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
}

raft_group_member_t *raft_hash_find(raft_hash_t *ht, raft_node_t *key) {
    unsigned int index = hash_function(key->value, ht->size);
    raft_hash_node_t *current = ht->table[index];
    while (current != NULL) {
        if (strcmp(current->key->value, key->value) == 0 && strlen(current->key->value) == strlen(key->value)) {
            return &(current->value);
        }
        current = current->next;
    }
    return NULL;
}

void raft_hash_delete(raft_hash_t *ht, raft_node_t *key) {
    unsigned int index = hash_function(key->value, ht->size);
    raft_hash_node_t *current = ht->table[index];
    raft_hash_node_t *prev = NULL;
    while (current != NULL) {
        if (strcmp(current->key->value, key->value) == 0) {
            if (prev == NULL) {
                ht->table[index] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

void raft_free_hash_table(raft_hash_t *ht) {
    for (int i = 0; i < ht->size; i++) {
        raft_hash_node_t *current = ht->table[i];
        while (current != NULL) {
            raft_hash_node_t *temp = current;
            current = current->next;
            free(temp);
        }
    }
    raft_free(ht->table, free);
    raft_free(ht, free);
}
