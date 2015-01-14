#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "utils.h"

#define PORT 7777

int main(int argc, char *argv[]) {
    int listener = 0, pid = getpid();
    struct sockaddr_in addr;

    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "server(%d): error creating listener\n", pid);
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    int so_reuseaddr = 1;

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr)) < 0) {
        fprintf(stderr, "server(%d): error calling setsockopt()\n", pid);
        exit(EXIT_FAILURE);
    }

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "server(%d): error calling bind()\n", pid);
        exit(EXIT_FAILURE);
    }

    if (listen(listener, 10) < 0) {
        fprintf(stderr, "server(%d): error calling listen()\n", pid);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "starting server\n");
    while (1) {
        int client;
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        if ((client = accept(listener, (struct sockaddr*)&client_addr, &addr_len)) < 0) {
            fprintf(stderr, "server(%d): error calling accept()\n", pid);
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, "server(%d): client(%s) connected\n", pid, inet_ntoa(client_addr.sin_addr));
        int pipe = connect_pipe(WORKER_SOCK);
        if (pipe < 0) {
            fprintf(stderr, "server(%d): error connecting pipe", pid);
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, "server(%d): sending socket to backend\n", pid);
        send_fd(pipe, client);
        close(client);
    }

    close(listener);
    return 0;
}
