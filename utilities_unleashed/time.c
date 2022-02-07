/**
 * utilities_unleashed
 * CS 241 - Spring 2022
 */
#include "format.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) print_time_usage();

    pid_t child = fork();
    if (child == -1) print_fork_failed();
    
    if (!child) { // child running
        execvp(argv[1], argv+1);
        print_exec_failed();
    } else {
        struct timespec start, stop;
        int s;
        clock_gettime(CLOCK_MONOTONIC, &start);
        waitpid(child, &s, 0);
        if (WIFEXITED(s) && WEXITSTATUS(s) == 0) {
            clock_gettime(CLOCK_MONOTONIC, &stop);
            display_results(argv, (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1000000000);
        }
    }
    return 0;
}
