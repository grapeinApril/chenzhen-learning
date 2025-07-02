#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#define SOCKET_PATH "/tmp/fd_socket"

// 接收文件描述符的函数
int recv_fd(int sockfd) {
    struct msghdr msg = {0};
    char ctrl_buf[CMSG_SPACE(sizeof(int))];
    char data_buf[1] = {0};
    int fd = -1;

    // 设置数据部分
    struct iovec iov = {
        .iov_base = data_buf,
        .iov_len = sizeof(data_buf)
    };
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // 设置辅助数据缓冲区
    msg.msg_control = ctrl_buf;
    msg.msg_controllen = sizeof(ctrl_buf);

    // 接收消息
    if (recvmsg(sockfd, &msg, 0) == -1) {
        perror("recvmsg");
        return -1;
    }

    // 解析辅助数据，获取文件描述符
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg && cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        if (cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
            memcpy(&fd, CMSG_DATA(cmsg), sizeof(int));
        }
    }

    return fd;
}

int main() {
    int sockfd;
    struct sockaddr_un addr;
    char buffer[256];
    ssize_t bytes_read;

    // 创建 Unix 域套接字
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return 1;
    }

    // 准备地址结构并连接
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    printf("正在连接到服务器...\n");
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    printf("已连接，等待接收文件描述符...\n");

    // 接收文件描述符
    int received_fd = recv_fd(sockfd);
    if (received_fd == -1) {
        close(sockfd);
        return 1;
    }

    printf("成功接收到文件描述符 (fd=%d)\n", received_fd);
    printf("读取文件内容...\n");

    // 使用接收到的文件描述符读取数据
    while ((bytes_read = read(received_fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("读取到的数据:\n%s", buffer);
    }

    if (bytes_read == -1) {
        perror("read");
    }

    // 清理资源
    close(received_fd);
    close(sockfd);

    return 0;
}