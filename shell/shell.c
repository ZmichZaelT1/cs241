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
static int command_failed = 0;

void load_file(char*, vector*);
// void print_vector(vector*);
void run_process(char*, int);
char** parse_string(char*, char*);
int cd(char*);
// void run_file_process(char*);
void write_vec_to_file(vector* his_vec, char* file_path);
void print_history();
char* find_his(char*);
int contains_logic(char*);
void run_and(char*);
char** splitString(char*, char*);
void run_or(char*);
int run(char*, char**);
void run_sep(char*);
void handle_sig();

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    if (argc != 3 && argc != 1) {print_usage(); exit(1);};

    his_vec = string_vector_create();
    todo_vec = string_vector_create();

    pid = getpid();
    int user_input_result = 1;
    size_t buffer_size;
    char *buffer = NULL;
    int arg;
    int count = 0;

    signal(SIGINT, handle_sig);

    if((arg=getopt(argc, argv,":f:h:")) != -1) {
        if (arg == 'h') {
            h=1;
            load_file(optarg, his_vec);
        } else if (arg == 'f') {
            load_file(optarg, todo_vec);
        } else {
            print_usage();
            exit(1);
        }
    }

    while (user_input_result) {
        // print_vector(todo_vec);
        if (!vector_empty(todo_vec)) {
            for (size_t i = 0; i < vector_size(todo_vec); i++) {
                run_process((char*) vector_get(todo_vec, i), 1);
            }
            vector_clear(todo_vec);
            exit(0);
        } else {
            if (buffer) {
                buffer[strlen(buffer)-1] = 0;
                run_process(buffer, 0);
            }

            getcwd(cwd, sizeof(cwd));
            print_prompt(cwd, pid);
        }
        user_input_result = getline(&buffer, &buffer_size, stdin);
        if (user_input_result == -1) exit(0);
        count++;
    }
    return 0;
}

void handle_sig() {
    return;
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
        if (h) {
            FILE *fd_new = fopen(file, "w");
            fclose(fd_new);
        } else {
            print_script_file_error();
            exit(1);
        }
        return;
    }
}

void run_process(char* process_args, int f) {
    if (!strcmp(process_args, "exit")) exit(0);
    if (!strcmp(process_args, "")) {
        vector_push_back(his_vec, process_args);
        if (h) write_vec_to_file(his_vec, file_path);
        return;
    }
    char *arg = process_args;

    if (contains_logic(arg)) {
        vector_push_back(his_vec, arg);
        if (h) write_vec_to_file(his_vec, file_path);
        return;
    }

    if (arg[0] == '#') {
        int nth = atoi(arg+1);
        if (nth < (int) vector_size(his_vec)) {
            arg = (char*) vector_get(his_vec, nth);
            print_command(arg);
            run_process(arg, f);
            return;
        } else {
            print_invalid_index();
            return;
        }
    }
    if (arg[0] == '!' && strcmp(arg, "!history")) {
        arg = find_his(arg+1);
        if (!arg) return;
        print_command(arg);
        run_process(arg, f);
        return;
    }

    char** argv = parse_string(arg, " ");

    vector_push_back(his_vec, arg);
    getcwd(cwd, sizeof(cwd));
    if (f) {print_prompt(cwd, pid); print_command(arg);}

    if (!strcmp(argv[0], "cd")) {
        cd(argv[1]);
        if (h) write_vec_to_file(his_vec, file_path);
        return;
    }
    if (!strcmp(argv[0], "!history")) {
        vector_pop_back(his_vec);
        print_history();
        return;
    }
    if (h) write_vec_to_file(his_vec, file_path);

    run(argv[0], argv);
    // printf("command: %d\n", command_failed);
}

void write_vec_to_file(vector* his_vec, char* file_path) {
    FILE* fd = fopen(file_path, "w+");
    size_t i = 0;
    for(; i < vector_size(his_vec); i++){
        fprintf(fd, "%s\n", (char*) vector_get(his_vec, i));
    }
    fclose(fd);
}

// failed: 0,  success: 1
int cd(char* dest) {
    int s = chdir(dest);
    if (s) {
        print_no_directory(dest);
        return 0;
    }
    return 1;
}

char** parse_string(char* str, char* deli) {
    char *tmp = strdup(str); // free this
    char *tmptmp = tmp;
    char *split = strtok(tmptmp, deli);
    int count = 1;
    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] == ' ') count++;
    }
    char **ret = calloc(count+1, sizeof(char*));
    for (int i = 0; i < count; i++) {
        ret[i] = strdup(split);
        split = strtok(NULL, deli);
    }
    free(tmp);
    return ret;
}

// void print_vector(vector* vec) {
//     for (size_t i = 0; i < vector_size(vec); i++) {
//         printf("%zuth element in vector is: %s\n", i, (char*) vector_get(vec, i));
//     }
// }

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
    for (int i = (int)vector_size(his_vec)-1; i >= 0; i--) {
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

int contains_logic(char* arg) {
    if (strstr(arg, "&&")) {
        run_and(arg); 
        return 1;
    }
    if (strstr(arg, "||")) {run_or(arg); return 1;}
    if (strstr(arg, ";")) {run_sep(arg); return 1;}
    return 0;
}

void run_sep(char* arg) {
    char** argv = splitString(arg, ";");
    char* process1 = argv[0];
    char* process2 = argv[1];

    if (process1[strlen(process1) - 1] == ' ') process1[strlen(process1) - 1] = '\0';
    if (process2[0] == ' ') strcpy(process2, process2+1);

    char** process1_argv = parse_string(process1, " ");
    char** process2_argv = parse_string(process2, " ");

// cd || cd
    if (!strcmp(process1_argv[0], "cd") && !strcmp(process2_argv[0], "cd")) {
        cd(process1_argv[1]);
        cd(process2_argv[1]);
        return;
    } 
// cd || others
    else if (!strcmp(process1_argv[0], "cd")) {
        cd(process1_argv[1]);
        run(process2_argv[0], process2_argv);
        return;
    } 
// others || cd
    else if (!strcmp(process2_argv[0], "cd")) {
        run(process1_argv[0], process1_argv);
        cd(process2_argv[1]);
        return;
    } 
// others || others
    else {
        run(process1_argv[0], process1_argv);
        run(process2_argv[0], process2_argv);
        return;
    }
}

void run_or(char* arg) {
    char** argv = splitString(arg, "||");
    char* process1 = argv[0];
    char* process2 = argv[1];

    if (process1[strlen(process1) - 1] == ' ') process1[strlen(process1) - 1] = '\0';
    if (process2[0] == ' ') strcpy(process2, process2+1);

    char** process1_argv = parse_string(process1, " ");
    char** process2_argv = parse_string(process2, " ");

// cd || cd
    if (!strcmp(process1_argv[0], "cd") && !strcmp(process2_argv[0], "cd")) {
        int scd1 = cd(process1_argv[1]);
        if (!scd1) cd(process2_argv[1]);
        return;
    } 
// cd || others
    else if (!strcmp(process1_argv[0], "cd")) {
        int scd1 = cd(process1_argv[1]);
        if (!scd1) run(process2_argv[0], process2_argv);
        return;
    } 
// others || cd
    else if (!strcmp(process2_argv[0], "cd")) {
        run(process1_argv[0], process1_argv);
        if (command_failed) cd(process2_argv[1]);
        return;
    } 
// others || others
    else {
        run(process1_argv[0], process1_argv);
        if (command_failed) run(process2_argv[0], process2_argv);
        return;
    }
}


// void cd(char* dest) {
//     int s = chdir(dest);
//     if (s) print_no_directory(dest);
// }
void run_and(char* arg) {
    char** argv = splitString(arg, "&&");
    char* process1 = argv[0];
    char* process2 = argv[1];

    if (process1[strlen(process1) - 1] == ' ') process1[strlen(process1) - 1] = '\0';
    if (process2[0] == ' ') strcpy(process2, process2+1);

    char** process1_argv = parse_string(process1, " ");
    char** process2_argv = parse_string(process2, " ");

// cd && cd
    if (!strcmp(process1_argv[0], "cd") && !strcmp(process2_argv[0], "cd")) {
        int scd1 = cd(process1_argv[1]);
        if (scd1) cd(process2_argv[1]);
        return;
    } 
// cd && others
    else if (!strcmp(process1_argv[0], "cd")) {
        int scd1 = cd(process1_argv[1]);
        if (scd1) run(process2_argv[0], process2_argv);
        return;
    } 
// others && cd
    else if (!strcmp(process2_argv[0], "cd")) {
        run(process1_argv[0], process1_argv);
        if (!command_failed) cd(process2_argv[1]);
        return;
    } 
// others  && others
    else {
        run(process1_argv[0], process1_argv);
        if (!command_failed) run(process2_argv[0], process2_argv);
        return;
    }
}

char** splitString(char* str, char* deli) {
    char **ret = calloc(3, sizeof(char*));
    char *tmp = strdup(str);

    char* tok = strtok(tmp, deli);
    ret[0] = strdup(tok);
    tok = strtok(NULL, deli);
    ret[1] = strdup(tok);
    free(tmp);
    return ret;
}

// failed: 0,  success: 1
int run(char*a, char** argv) {
    fflush(stdin);
    fflush(stdout);
    pid_t child = fork();
    if (child == -1) {
        print_fork_failed();
        command_failed = 1;
        return 0;
    }
    if (!child) {
        print_command_executed(getpid());
        execvp(a, argv);
        print_exec_failed(a);
        exit(1);
    } else {
        command_failed = 0;
        int s;
        int ws = waitpid(child, &s, 0);
        if (ws == -1) print_wait_failed();
        if (WIFEXITED(s) && WEXITSTATUS(s) != 0) {
            // print_exec_failed(a);
            command_failed = 1;
            return 0;
        }
    }
    return 1;
}
