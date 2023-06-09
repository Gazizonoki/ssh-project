#include "send_spawn.h"

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>  
#include <sys/types.h> 
#include <sys/socket.h>
#include <unistd.h>

int create_socket(char* hostname, char* port) {
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    };

    int err;
    struct addrinfo* res;
    if ((err = getaddrinfo(hostname, port, &hints, &res)) != 0) {
        fputs(gai_strerror(err), stderr);
        exit(1);
    }

    int sock = -1;
    for (struct addrinfo* info = res; info; info = info->ai_next) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            continue;
        }
        if (connect(sock, info->ai_addr, info->ai_addrlen)) {
            close(sock);
            sock = -1;
            continue;
        }
        if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK)) {
            close(sock);
            sock = -1;
            continue;
        }
        break;
    }
    return sock;
}

enum { BUFFER_SIZE = 1000 };

static unsigned char buf[BUFFER_SIZE];

ssize_t transfer_data(int src_fd, int dst_fd) {
    ssize_t len;
    while ((len = read(src_fd, buf, BUFFER_SIZE)) > 0) {
        write(dst_fd, buf, len);
    }
    return len;
}

void spawn_cmd(char* hostname, char* port, int argc, char** argv) {
    int sock = create_socket(hostname, port);
    if (sock == -1) {
        fputs("Сouldn't create socket\n", stderr);
        exit(1);
    }
    if (fcntl(STDIN_FILENO, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK)) {
        exit(1);
    }
    uint32_t arg_cnt = htonl(argc);
    write(sock, &arg_cnt, sizeof(arg_cnt));

    for (size_t i = 0; i < argc; ++i) {
        uint32_t len = htonl(strlen(argv[i]));
        write(sock, &len, sizeof(len));
        write(sock, argv[i], strlen(argv[i]) * sizeof(*argv[i]));
    }

    struct pollfd pfd[2];
    pfd[0].fd = STDIN_FILENO;
    pfd[0].events = POLLIN;

    pfd[1].fd = sock;
    pfd[1].events = POLLIN;

    while (true) {
        if (poll(pfd, 2, -1) == -1) {
            exit(1);
        }
        if ((pfd[1].revents & POLLERR) || (pfd[1].revents & POLLHUP)) {
            break;
        }
        if (pfd[0].revents & POLLIN) {
            if (transfer_data(STDIN_FILENO, sock) == 0) {
                const unsigned char end_symbol = 255;
                write(sock, &end_symbol, 1);
            }
        }
        if (pfd[1].revents & POLLIN) {
            ssize_t len = transfer_data(sock, STDOUT_FILENO);
            if (len == 0 || (len == -1 && errno != EAGAIN)) {
                break;
            }
        }
    }
    close(sock);
}