#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <liburing.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 128

// 连接上下文结构体
struct conn_data {
    int fd;
    char buffer[BUFFER_SIZE];
};

// 设置 socket 并监听
int setup_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, SOMAXCONN) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// 提交 accept 请求
void submit_accept(struct io_uring *ring, int server_fd) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    if (!sqe) {
        fprintf(stderr, "Failed to get SQE\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in *client_addr = malloc(sizeof(struct sockaddr_in));
    socklen_t *client_addr_len = malloc(sizeof(socklen_t));
    *client_addr_len = sizeof(struct sockaddr_in);

    io_uring_prep_accept(sqe, server_fd, (struct sockaddr *)client_addr, 
                         client_addr_len, 0);
    io_uring_sqe_set_data(sqe, client_addr);  // 保存客户端地址指针

    io_uring_submit(ring);
}

// 提交 read 请求
void submit_read(struct io_uring *ring, int client_fd) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    if (!sqe) {
        fprintf(stderr, "Failed to get SQE\n");
        exit(EXIT_FAILURE);
    }

    struct conn_data *conn = malloc(sizeof(struct conn_data));
    conn->fd = client_fd;

    io_uring_prep_recv(sqe, client_fd, conn->buffer, BUFFER_SIZE, 0);
    io_uring_sqe_set_data(sqe, conn);

    io_uring_submit(ring);
}

// 提交 write 请求
void submit_write(struct io_uring *ring, struct conn_data *conn, size_t len) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    if (!sqe) {
        fprintf(stderr, "Failed to get SQE\n");
        exit(EXIT_FAILURE);
    }

    io_uring_prep_send(sqe, conn->fd, conn->buffer, len, 0);
    io_uring_sqe_set_data(sqe, conn);

    io_uring_submit(ring);
}

int main() {
    struct io_uring ring;
    int server_fd;

    // 初始化 io_uring
    if (io_uring_queue_init(MAX_EVENTS, &ring, 0) < 0) {
        perror("io_uring_queue_init failed");
        exit(EXIT_FAILURE);
    }

    // 设置 socket
    server_fd = setup_socket();
    printf("Server listening on port %d...\n", PORT);

    // 提交第一个 accept 请求
    submit_accept(&ring, server_fd);

    // 事件循环
    while (1) {
        struct io_uring_cqe *cqe;
        int ret = io_uring_wait_cqe(&ring, &cqe);
        if (ret < 0) {
            perror("io_uring_wait_cqe failed");
            exit(EXIT_FAILURE);
        }

        if (cqe->res < 0) {
            fprintf(stderr, "I/O operation failed: %d\n", cqe->res);
        } else {
            // 获取用户数据
            void *user_data = io_uring_cqe_get_data(cqe);

            if (user_data == NULL) {
                fprintf(stderr, "Invalid user data\n");
                io_uring_cqe_seen(&ring, cqe);
                continue;
            }

            // 处理 accept 完成事件
            if (cqe->res > 0) {  // accept 返回新的客户端 fd
                struct sockaddr_in *client_addr = user_data;
                int client_fd = cqe->res;

                printf("New connection from %s:%d\n",
                       inet_ntoa(client_addr->sin_addr),
                       ntohs(client_addr->sin_port));

                free(client_addr);  // 释放 accept 时分配的内存

                // 提交新的 accept 请求
                submit_accept(&ring, server_fd);

                // 提交 read 请求给新连接
                submit_read(&ring, client_fd);
            }
            // 处理 read 完成事件
            else if (cqe->res > 0) {  // read 返回读取的字节数
                struct conn_data *conn = user_data;
                size_t len = cqe->res;

                printf("Read %zu bytes from client %d\n", len, conn->fd);

                // 提交 write 请求回显数据
                submit_write(&ring, conn, len);
            }
            // 处理 write 完成事件
            else {
                struct conn_data *conn = user_data;
                printf("Write completed for client %d\n", conn->fd);
                close(conn->fd);
                free(conn);
            }
        }

        // 标记 CQE 已处理
        io_uring_cqe_seen(&ring, cqe);
    }

    close(server_fd);
    io_uring_queue_exit(&ring);
    return 0;
}