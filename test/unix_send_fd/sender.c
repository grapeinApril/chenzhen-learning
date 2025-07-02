#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#define SOCKET_PATH "/tmp/fd_socket"

// 发送文件描述符的函数
int send_fd(int sockfd, int fd_to_send) {
    struct msghdr msg = {0};
    struct cmsghdr *cmsg;
    char ctrl_buf[CMSG_SPACE(sizeof(int))];
    char data_buf[1] = {0};

    // 设置数据部分（必须有数据，否则辅助数据不会被发送）
    struct iovec iov = {
        .iov_base = data_buf,
        .iov_len = sizeof(data_buf)
    };
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // 设置辅助数据（携带文件描述符）
    msg.msg_control = ctrl_buf;
    msg.msg_controllen = sizeof(ctrl_buf);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    memcpy(CMSG_DATA(cmsg), &fd_to_send, sizeof(int));

    // 发送消息
    if (sendmsg(sockfd, &msg, 0) == -1) {
        perror("sendmsg");
        return -1;
    }
    return 0;
}

int main() {
    int sockfd, connfd;
    struct sockaddr_un addr;
    int fd_to_send;

    // 创建 Unix 域套接字
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return 1;
    }

    // 准备地址结构并绑定
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // 删除可能存在的旧套接字文件
    unlink(SOCKET_PATH);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    // 监听连接
    if (listen(sockfd, 1) == -1) {
        perror("listen");
        close(sockfd);
        return 1;
    }

    printf("等待客户端连接...\n");
    connfd = accept(sockfd, NULL, NULL);
    if (connfd == -1) {
        perror("accept");
        close(sockfd);
        return 1;
    }

    printf("客户端已连接，准备发送文件描述符...\n");

    // 打开一个文件（这里打开 /etc/passwd 作为示例）
    fd_to_send = open("/etc/passwd", O_RDONLY);
    if (fd_to_send == -1) {
        perror("open");
        close(connfd);
        close(sockfd);
        return 1;
    }

    // 发送文件描述符
    if (send_fd(connfd, fd_to_send) == -1) {
        close(fd_to_send);
        close(connfd);
        close(sockfd);
        return 1;
    }

    printf("文件描述符已发送 (fd=%d)\n", fd_to_send);

    // 清理资源
    close(fd_to_send);
    close(connfd);
    close(sockfd);
    unlink(SOCKET_PATH);

    return 0;
}