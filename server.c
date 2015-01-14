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
    int listener = 0;
    struct sockaddr_in addr;

    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "error creating listener\n");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    int so_reuseaddr = 1;

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr)) < 0) {
        fprintf(stderr, "error calling setsockopt()\n");
        exit(EXIT_FAILURE);
    }

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "error calling bind()\n");
        exit(EXIT_FAILURE);
    }

    if (listen(listener, 10) < 0) {
        fprintf(stderr, "error calling listen()\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "starting server\n");
    while (1) {
        int client;
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        if ((client = accept(listener, (struct sockaddr*)&client_addr, &addr_len)) < 0) {
            fprintf(stderr, "error calling accept()\n");
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, "client connected: %d\n", client);

        int pipe = connect_pipe(WORKER_SOCK);
        if (pipe < 0) {
            fprintf(stderr, "error connecting pipe: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        fprintf(stdout, "sending client(%d) to backend(%d)\n", client, pipe);
        send_fd(pipe, client);
        close(client);
    }

    close(listener);
    return 0;
}
