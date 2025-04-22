/**
 * raft_utils.c
 * @author chenzhen
 * 
 */
#include "raft_utils.h"

void *raft_malloc(size_t size) {
    void *ptr = NULL;

    if (size <= 0)
        return ptr;

    ptr = malloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

void raft_free(void *ptr, free_func free_func) {
    if (ptr) {
        free_func(ptr);
        ptr = NULL;
    }
}