#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "utils.h"

int read_all(int sock, unsigned int size, void *buffer) {
    unsigned int bytes_read = 0;
    int result;
    while (bytes_read < size) {
        result = read(sock, buffer + bytes_read, size - bytes_read);
        if (result < 1) {
            return -1;
        }
        bytes_read += result;
    }
    return bytes_read;
}

int handler(int sock) {
    int pid = getpid();
    fprintf(stdout, "handler(%d): waiting data(%d)\n", pid, sock);
    while (1) {
        char buf[5];
        int read_total = read_all(sock, 5, buf);
        if (read_total < 0) {
            close(sock);
            break;
        }
        fprintf(stdout, "handler(%d): received data: %s\n", pid, buf);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int pid = getpid();
    fprintf(stdout, "worker(%d): starting\n", pid);

    fprintf(stdout, "worker(%d): creating pipe\n", pid);
    int pipe = create_pipe();
    if (pipe < 0) {
        fprintf(stderr, "worker(%d): %s", pid, strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (1) {
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(struct sockaddr_un));
        socklen_t addr_len = 0;
        int conn_fd = accept(pipe, (struct sockaddr *)&addr, &addr_len);
        int sock_fd = recv_fd(conn_fd);
        if (sock_fd < 0) {
            fprintf(stderr, "worker(%d): error reading socket descriptor\n", pid);
            continue;
        }
        int pid = fork();
        if (pid == 0) {
            return handler(sock_fd);
        }
        close(conn_fd);
    }
    return 0;
}
