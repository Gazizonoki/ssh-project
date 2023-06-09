#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

enum { BUFFER_SIZE = 1000 };

void exec_command(int conn) {
    uint32_t arg_count, tmp;
    read(conn, &tmp, sizeof(arg_count));
    arg_count = ntohl(tmp);
    char* args[arg_count + 1];
    args[arg_count] = NULL;
    for (size_t i = 0; i < arg_count; ++i) {
        uint32_t str_size;
        read(conn, &tmp, sizeof(tmp));
        str_size = ntohl(tmp);
        args[i] = calloc(str_size + 1, sizeof(*args[i]));
        read(conn, args[i], str_size);
    }

    int in_pipe[2];
    pipe(in_pipe);

    int pid = fork();
    if (pid == 0) {
        dup2(conn, STDOUT_FILENO);
        dup2(in_pipe[0], STDIN_FILENO);
        close(conn);
        close(in_pipe[1]);
        execvp(args[0], args);
        for (size_t i = 0; i < arg_count; ++i) {
            free(args[i]);
        }
        exit(1);
    }
    close(in_pipe[0]);
    for (size_t i = 0; i < arg_count; ++i) {
        free(args[i]);
    }
    unsigned char c;
    while (read(conn, &c, 1) > 0) {
        if (c == 255) {
            break;
        }
        write(in_pipe[1], &c, 1);
    }
    close(in_pipe[1]);
    close(conn);
}