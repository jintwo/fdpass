#ifndef _UTILS_H
#define _UTILS_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define WORKER_SOCK "/tmp/server-worker.sock"

int create_pipe() {
    struct sockaddr_un addr;
    int fd;

    if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "error calling socket()\n");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));

    addr.sun_family = AF_UNIX;
    unlink(WORKER_SOCK);
    strcpy(addr.sun_path, WORKER_SOCK);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
        fprintf(stderr, "error calling bind()\n");
        exit(EXIT_FAILURE);
    }

    if (listen(fd, 10) < 0) {
        fprintf(stderr, "error calling listen()\n");
        exit(EXIT_FAILURE);
    }

    return fd;
}

int connect_pipe(char *pipe_path) {
    struct sockaddr_un addr;
    int fd;

    if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "error calling socket()\n");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, pipe_path);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
        fprintf(stderr, "error calling connect()\n");
        exit(EXIT_FAILURE);
    }

    return fd;
}

int recv_fd(int pipe) {
    int sent_fd;
    struct msghdr message;
    struct iovec iov[1];
    struct cmsghdr *control_message;
    char data[1];
    char ctrl_buf[CMSG_SPACE(sizeof(int))];

    memset(&message, 0, sizeof(struct msghdr));
    memset(ctrl_buf, 0, CMSG_SPACE(sizeof(int)));

    iov[0].iov_base = data;
    iov[0].iov_len = 1;
    message.msg_iov = iov;
    message.msg_iovlen = 1;

    message.msg_control = ctrl_buf;
    message.msg_controllen = CMSG_SPACE(sizeof(int));
    message.msg_name = NULL;
    message.msg_namelen = 0;

    if (recvmsg(pipe, &message, 0) < 0) {
        return -1;
    }

    if (data[0] != ' ') {
        return -1;
    }

    if ((message.msg_flags & MSG_CTRUNC) == MSG_CTRUNC) {
        return -1;
    }

    for (control_message = CMSG_FIRSTHDR(&message);
         control_message != NULL;
         control_message = CMSG_NXTHDR(&message, control_message)) {
        if ((control_message->cmsg_level == SOL_SOCKET) &&
            (control_message->cmsg_type == SCM_RIGHTS)) {
            sent_fd = *((int *)CMSG_DATA(control_message));
            return sent_fd;
        }
    }

    return -1;
}

int send_fd(int pipe, int fd) {
    struct msghdr message;
    struct iovec iov[1];
    struct cmsghdr *control_message = NULL;
    char data[1];
    char ctrl_buf[CMSG_SPACE(sizeof(int))];

    data[0] = ' ';
    iov[0].iov_base = data;
    iov[0].iov_len = sizeof(data);

    memset(&message, 0, sizeof(struct msghdr));
    message.msg_iov = iov;
    message.msg_iovlen = 1;

    memset(ctrl_buf, 0, CMSG_SPACE(sizeof(int)));
    message.msg_control = ctrl_buf;
    message.msg_controllen = CMSG_SPACE(sizeof(int));
    message.msg_name = NULL;
    message.msg_namelen = 0;

    control_message = CMSG_FIRSTHDR(&message);
    control_message->cmsg_level = SOL_SOCKET;
    control_message->cmsg_type = SCM_RIGHTS;
    control_message->cmsg_len = CMSG_LEN(sizeof(int));
    *((int *)CMSG_DATA(control_message)) = fd;

    return sendmsg(pipe, &message, 0);
}
#endif
