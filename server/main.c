#include "listener.h"

#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        exit(1);
    }

    if (!pid) {
        setsid();
        start_listen(argv[1]);
    }
}