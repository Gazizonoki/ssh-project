#include "send_spawn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        fputs("Missing argument: server address\n", stderr);
        exit(1);
    }
    if (argc < 3) {
        fputs("Missing argument: command\n", stderr);
        exit(1);
    }
    size_t pos = 0;
    size_t len = strlen(argv[1]);
    while (pos < len && argv[1][pos] != ':') {
        ++pos;
    }
    if (pos == len) {
        fputs("Address should have the following form: 'HOSTNAME:PORT'\n", stderr);
        exit(1);
    }
    char hostname[pos + 1];
    char port[len - pos];
    hostname[pos] = '\0';
    port[len - 1 - pos] = '\0';
    memcpy(hostname, argv[1], pos);
    memcpy(port, argv[1] + pos + 1, len - 1 - pos);
    if (strcmp(argv[2], "spawn") == 0) {
        spawn_cmd(hostname, port, argc - 3, argv + 3);
    } else {
        fputs("Unknown command\n", stderr);
        exit(1);
    }
    return 0;
}