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

void str_cli(FILE *fp, int sockfd)
{
    char sendline[1024], recvline[1024];

    while (fgets(sendline, sizeof(sendline) - 1, fp) != NULL) {
        write(sockfd, sendline, strlen(sendline));

        if (read(sockfd, recvline, sizeof(recvline)) == 0) {
            printf("server closed\n");
            return; 
        }

        fputs(recvline, fp);
    }
}

int main(int argc, char* argv[]) 
{
    struct sockaddr_in server_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printf("socket error: %s\n", strerror(errno));
        exit(0);
    }

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9877);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("connect error: %s\n", strerror(errno));
        exit(0);
    }

    str_cli(stdin, sockfd);
}