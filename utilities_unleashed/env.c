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
#include <string.h>
char **seperate_variable(char*);
int main(int argc, char *argv[]) {
    if (argc < 3) print_env_usage();
    pid_t child = fork();
    if (child == -1) print_fork_failed();
    if (child) {
        int s;
        waitpid(child, &s, 0);
    } else {
        char **tmp = argv;
        int count = argc - 3;
        tmp++;
        while (*tmp && count >= 0) {
            if (strcmp(*tmp, "--")) {
                char **key_value = seperate_variable(*tmp);
                if (key_value[1]) {
                    int set_env = 0;
                    if (key_value[1][0] == '%') {
                        char* ref_env = getenv(key_value[1]+1);
                        if (ref_env) {
                            set_env = setenv(key_value[0], getenv(key_value[1]+1), 1);
                        } else {
                            set_env = setenv(key_value[0], key_value[1], 1);
                        }
                        if(set_env == -1) print_environment_change_failed();
                    } else {
                        set_env = setenv(key_value[0], key_value[1], 1);
                        if(set_env == -1) print_environment_change_failed();
                    }

                } else {
                    print_env_usage();
                }
            } else {
                execvp(tmp[1], tmp+1);
                print_exec_failed();
                return 0;
            }
            count--;
            tmp++;
        }
    }
    return 0;
}

// input a string, output a array of string seperate by "="
char **seperate_variable(char* variable) {
    char **ret = malloc(2 * sizeof(char*));
    char* tmp = strdup(variable);
    char* split = strsep(&tmp, "=");
    ret[0] = split;
    ret[1] = tmp;
    return ret;
}