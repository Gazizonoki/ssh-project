#include "listener.h"
#include "spawn.h"

#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <unistd.h>

int create_socket(char* port) {
    const struct addrinfo hints = {
        .ai_flags = AI_PASSIVE,
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    };

    int err;
    struct addrinfo* res;
    if ((err = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        exit(1);
    }

    int sock = -1;
    for (struct addrinfo* info = res; info; info = info->ai_next) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            continue;
        }
        if (bind(sock, info->ai_addr, info->ai_addrlen) == -1) {
            close(sock);
            sock = -1;
            continue;
        }
        if (listen(sock, SOMAXCONN) == -1) {
            close(sock);
            sock = -1;
            continue;
        }
        break;
    }
    return sock;
}

void sig_handler(int) {
    waitpid(-1, NULL, WNOHANG);
}

void start_listen(char* port) {
    int sock = create_socket(port);
    if (sock == -1) {
        exit(1);
    }
    struct sigaction act;
    act.sa_handler = sig_handler;
    sigemptyset(&act.sa_mask);
    sigaction(SIGCHLD, &act, NULL);

    while (true) {
        int conn = accept(sock, NULL, 0);
        pid_t pid = fork();
        if (pid == 0) {
            close(sock);
            exec_command(conn);
            exit(0);
        }
        close(conn);
    }
    close(sock);
}