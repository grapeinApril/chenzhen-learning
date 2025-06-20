#ifndef RAFT_MACROS_H
#define RAFT_MACROS_H

/* 最小选举时间间隔 */
#define MIN_ELECTION_TIMEOUT        3000  //ms

/* 最大选举时间间隔 */ 
#define MAX_ELECTION_TIMEOUT        4000  //ms

/* 日志复制间隔 */
#define LOG_REPLICATION_INTERVAL    1000  //ms
#endif