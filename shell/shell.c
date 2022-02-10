/**
 * shell
 * CS 241 - Spring 2022
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>

typedef struct process {
    char *command;
    pid_t pid;
} process;

extern char *optarg;
extern int optind, opterr, optopt;
static vector *his_vec;
static vector *todo_vec;
static char cwd[PATH_MAX];
static int pid;

void load_file(char*, vector*);
void print_vector(vector*);
void run_process(char*);
char** parse_string(char*);
void cd(char*);


int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    if (argc >3) {print_usage(); exit(1);};

    his_vec = string_vector_create();
    todo_vec = string_vector_create();

    pid = getpid();
    int user_input_result;
    size_t buffer_size;
    char *buffer;
    int arg;



    if((arg=getopt(argc, argv,":f:h:")) != -1) {
        if (arg == 'h') {
            load_file(optarg, his_vec);
        } else if (arg == 'f') {
            load_file(optarg, todo_vec);

        }
    }

    while (1) {
        // print_vector(todo_vec);
        if (!vector_empty(todo_vec)) {
            for (size_t i = 0; i < vector_size(todo_vec); i++) {
                run_process((char*) vector_get(todo_vec, i));
            }
            vector_clear(todo_vec);
            getcwd(cwd, sizeof(cwd));
            print_prompt(cwd, pid);
            user_input_result = getline(&buffer, &buffer_size, stdin);
        } else {
            run_process(buffer);

            getcwd(cwd, sizeof(cwd));
            print_prompt(cwd, pid);
            user_input_result = getline(&buffer, &buffer_size, stdin);
        }

    }
    return 0;
}


void load_file(char *file, vector* vec) {
    FILE *fd = fopen(get_full_path(file), "r");
    if (fd) {
        char *buffer = NULL;
        size_t buffer_size = 0;
        while (getline(&buffer, &buffer_size, fd) != -1) {
            
            if (buffer[strlen(buffer) - 1] == '\n') {
                buffer[strlen(buffer) - 1] = '\0';
            }
            vector_push_back(vec, buffer);
        }
        fclose(fd);
    } else {
        print_script_file_error();
        fclose(fd);
        return;
    }
}

void run_process(char* process_args) {
    char** argv = parse_string(process_args);
    
    getcwd(cwd, sizeof(cwd));
    print_prompt(cwd, pid);
    print_command(process_args);

    if (!strcmp(argv[0], "cd")) {
        cd(argv[1]);
        return;
    }

    pid_t child = fork();
    if (child == -1) print_fork_failed();

    if (!child) { // child running
        print_command_executed(getpid());
        execvp(argv[0], argv);
        print_exec_failed(process_args);
    } else {
        int s;
        waitpid(child, &s, 0);
    }
}

void cd(char* dest) {
    int s = chdir(dest);
    if (s) print_no_directory(dest);
}

char** parse_string(char* str) {
    char *tmp = strdup(str); // free this
    char *tmptmp = tmp;
    char *split = strtok(tmptmp, " ");
    int count = 1;
    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] == ' ') count++;
    }
    char **ret = calloc(count+1, sizeof(char*));
    for (int i = 0; i < count; i++) {
        ret[i] = strdup(split);
        split = strtok(NULL, " ");
    }
    free(tmp);
    return ret;
}

void print_vector(vector* vec) {
    for (size_t i = 0; i < vector_size(vec); i++) {
        printf("%zuth element in vector is: %s\n", i, (char*) vector_get(vec, i));
    }
}