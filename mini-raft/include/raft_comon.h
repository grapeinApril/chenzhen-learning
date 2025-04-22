/**
 * raft common
 * @author chenzhen
 * 
 */
#ifndef RAFT_COMMON_H
#define RAFT_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "raft_hash.h"
#include <stddef.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* 服务器节点表示ID */
typedef struct raft_node_s {
    char* value;
    bool (*equal)(const raft_node_t *node1, const raft_node_t *node2);
    int (*hash)(const raft_node_t *value);
}raft_node_t;

typedef struct raft_replicating_state_s {

}raft_replicating_state_t;

typedef struct raft_node_endpoint_s {
    raft_node_t *id;
    char ipv4_addr[32];
    int port;
}raft_endpoint_t;

typedef struct raft_group_member_s {
    raft_endpoint_t *endpoint;
    raft_replicating_state_t *replicating_state;
}raft_group_member_t;

/* 集群成员表结构 */
typedef struct node_group_s {
    raft_hash_t *member_map;
    void (*buil_servers)(node_group_t *group, void *);
}node_group_t;

#endif