# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <errno.h>
# include <netinet/in.h>
# include <sys/socket.h>

void str_echo(int sockfd) {
    ssize_t n;
    char buf[1024] = { 0 };

    again:
    while ((n = read(sockfd, buf, sizeof(buf)) > 0)) {
        write(sockfd, buf, n);
        if (n < 0 && errno == EINTR) 
            goto again;
        else if (n < 0) {
            printf("str_echo: read error");
        }
    }
}

int main(int argc, char* argv[]) {
    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;
    pid_t childpid;
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(9877);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    listen(listenfd, 10);

    for (;;) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if (childpid = fork() == 0) {
            close(listenfd);

        }
        close(connfd);
    }
}