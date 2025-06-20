#include <stddef.h>
#ifndef HA_UTILS_H
#define HA_UTILS_H

typedef void (*free_func)(void *ptr);

void *raft_malloc(size_t size);

void raft_free(void *ptr, free_func free_func);
#endif HA_UTILS_H