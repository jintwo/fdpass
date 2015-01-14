#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
                      
#define MAXLINE 256

int main(int argc, char **argv) {
    int sockfd, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;    

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <IPAddress>", argv[0]);
        exit(0);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Can't create socket");
        exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(13);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error for %s", argv[1]);
        exit(0);
    }
    
    if (connect(sockfd, (sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "connect error");
        exit(0);
    }

    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            fprintf(stderr, "fputs error");
        }
    }

    if (n < 0) {
        fprintf(stderr, "read error");
        exit(0);
    }

    return 0;
}
