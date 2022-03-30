/**
 * mapreduce
 * CS 241 - Spring 2022
 */
#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 6) {
        print_usage();
        return 1;
    }
    char *input_file = argv[1];
    char *output_file = argv[2];
    char *mapper_executable = argv[3];
    char *reducer_executable = argv[4];
    int mapper_count = atoi(argv[5]);

    // Create an input pipe for each mapper.
    int pipe_map[mapper_count*2];
    for (int i = 0; i < mapper_count; i++) {
        if (pipe(pipe_map+i*2) < 0) return 1;
    }

    // Create one input pipe for the reducer.
    int pipe_red[2];
    if (pipe(pipe_red) < 0) return 1;

    // Open the output file.
    int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);

    // Start a splitter process for each mapper.
    pid_t childs[mapper_count];
    for (int i = 0; i < mapper_count; i++) {
        pid_t child = fork();
        childs[i] = child;
        if (child == 0) { //child+
            dup2(pipe_map[i*2 + 1], 1);
            close(pipe_map[i*2]);
            // char *id = itoa(i);
            char id[20];
            sprintf(id, "%d", i);
            execl("./splitter", "./splitter", input_file, argv[5], id, NULL);
            exit(1);
        }
    }

    // Start all the mapper processes.
    pid_t childs_map[mapper_count];
    for (int i = 0; i < mapper_count; i++) {
        close(pipe_map[i*2 + 1]);
        pid_t child = fork();
        childs_map[i] = child;
        if (child == 0) { //child
            dup2(pipe_map[i*2], 0);
            dup2(pipe_red[1], 1);
            close(pipe_red[0]);
            execl(mapper_executable, mapper_executable, NULL);
            exit(1);
        }
    }

    // Start the reducer process.
    close(pipe_red[1]);
    pid_t child = fork();
    if (child == 0) { //child
        dup2(pipe_red[0], 0);
        dup2(fd_out, 1);
        execl(reducer_executable, reducer_executable, NULL);
        exit(1);
    }

    // Wait for the reducer to finish.
    for (int i = 0; i < mapper_count; i++) {
        int s;
        waitpid(childs[i], &s, 0);
    }

    for (int i = 0; i < mapper_count; i++) {
        close(pipe_map[i*2]);
        int s;
        waitpid(childs_map[i], &s, 0);
    }
    int ss;
    waitpid(child, &ss, 0);

    // Print nonzero subprocess exit codes.
    if (ss) print_nonzero_exit_status(reducer_executable, ss);

    // Count the number of lines in the output file.
    print_num_lines(output_file);

    close(pipe_red[0]);
    close(fd_out);

    return 0;
}
