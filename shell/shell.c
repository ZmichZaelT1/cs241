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
static int h = 0;
static char *file_path;

void load_file(char*, vector*);
void print_vector(vector*);
void run_process(char*, int);
char** parse_string(char*);
void cd(char*);
// void run_file_process(char*);
void write_vec_to_file(vector* his_vec, char* file_path);
void print_history();
char* find_his(char*);


int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    if (argc >3) {print_usage(); exit(1);};

    his_vec = string_vector_create();
    todo_vec = string_vector_create();

    pid = getpid();
    int user_input_result;
    size_t buffer_size;
    char *buffer = NULL;
    int arg;
    int count = 0;


    if((arg=getopt(argc, argv,":f:h:")) != -1) {
        if (arg == 'h') {
            load_file(optarg, his_vec);
            h=1;
        } else if (arg == 'f') {
            load_file(optarg, todo_vec);

        }
    }

    while (count != 100) {
        // print_vector(todo_vec);
        if (!vector_empty(todo_vec)) {
            for (size_t i = 0; i < vector_size(todo_vec); i++) {
                run_process((char*) vector_get(todo_vec, i), 1);
            }
            vector_clear(todo_vec);
            getcwd(cwd, sizeof(cwd));
            print_prompt(cwd, pid);
            user_input_result = getline(&buffer, &buffer_size, stdin);
        } else {
            if (buffer) {
                buffer[strlen(buffer)-1] = 0;
                run_process(buffer, 0);
            }

            getcwd(cwd, sizeof(cwd));
            print_prompt(cwd, pid);
            user_input_result = getline(&buffer, &buffer_size, stdin);
        }
        count++;
    }
    return 0;
}


void load_file(char *file, vector* vec) {
    file_path = get_full_path(file);
    FILE *fd = fopen(file_path, "r");
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

void run_process(char* process_args, int f) {
    if (!strcmp(process_args, "exit")) exit(0);

    char *arg = process_args;

    if (process_args[0] == '#') {
        int nth = atoi(process_args+1);
        if (nth < (int) vector_size(his_vec)) {
            arg = (char*) vector_get(his_vec, nth);
        } else {
            print_invalid_index();
            return;
        }
    }
    if (process_args[0] == '!' && strcmp(process_args, "!history")) {
        arg = find_his(process_args+1);
        if (!arg) return;
    }

    char** argv = parse_string(arg);

    vector_push_back(his_vec, arg);
    if (h) write_vec_to_file(his_vec, file_path);
    getcwd(cwd, sizeof(cwd));
    if (f) {print_prompt(cwd, pid); print_command(arg);}

    if (!strcmp(argv[0], "cd")) {
        cd(argv[1]);
        return;
    }
    if (!strcmp(argv[0], "!history")) {
        vector_pop_back(his_vec);
        print_history();
        return;
    }
    fflush(stdin);
    pid_t child = fork();
    if (child == -1) print_fork_failed();

    if (!child) { // child running
        print_command_executed(getpid());
        execvp(argv[0], argv);
        print_exec_failed(arg);
    } else {
        int s;
        int ws = waitpid(child, &s, 0);
        if (ws == -1) print_wait_failed();
        if (!WIFEXITED(s) && WEXITSTATUS(s) != 0) print_exec_failed(process_args);
    }
}

void write_vec_to_file(vector* his_vec, char* file_path) {
    FILE* fd = fopen(file_path, "w+");
    size_t i = 0;
    for(; i < vector_size(his_vec); i++){
        fprintf(fd, "%s\n", (char*) vector_get(his_vec, i));
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

void print_history() {
    for (size_t i = 0; i < vector_size(his_vec); i++) {
        print_history_line(i, (char*) vector_get(his_vec, i));
    }
}

char* find_his(char* prefix) {
    if (!*prefix) {
        return vector_get(his_vec, vector_size(his_vec)-1);
    }
    size_t pre_length = strlen(prefix);
    for (size_t i = vector_size(his_vec)-1; i >= 0; i--) {
        int founded = 1;
        char* his = (char*) vector_get(his_vec, i);
        size_t his_length = strlen(his);
        if (pre_length > his_length) {
            print_no_history_match();
            return NULL;
        }
        size_t iter = (pre_length < his_length) ? pre_length : his_length;
        for (size_t j = 0; j < iter; j++) {
            if (prefix[j] != his[j]) {
                founded = 0;
                break;
            }
        }
        if (founded) return his;
    }
    print_no_history_match();
    return NULL;
}