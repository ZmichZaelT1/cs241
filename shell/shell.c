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
#include <errno.h>

typedef struct process {
    char *command;
    pid_t pid;
} process;

extern char *optarg;
extern int optind, opterr, optopt;
static vector *his_vec;
static vector *todo_vec;
static vector *processes_vec;
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
int run(char*, char**, char*);
void run_sep(char*);
void handle_sig();
int check_background(char**, int*);
void child_handler(int);
void run_ps();
process* add_process(char*, pid_t);
process_info *load_info(char*, pid_t);
int check_direction(char**, char*);
int run_OUTPUT(char**, char*, int);
char* find_path(char** argv);
int run_INPUT(char**, char*);
void kill_process(pid_t, int, char*);

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    if (argc != 3 && argc != 1) {print_usage(); exit(1);};

    his_vec = string_vector_create();
    todo_vec = string_vector_create();
    processes_vec = shallow_vector_create();

    process *main = add_process(argv[0], getpid());
    vector_push_back(processes_vec, main);

    pid = getpid();
    int user_input_result = 1;
    size_t buffer_size;
    char *buffer = NULL;
    int arg;
    int count = 0;

    signal(SIGINT, handle_sig);
    // signal(SIGCHLD, child_handler);

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

process* add_process(char* command, pid_t pid) {
    process *proc = calloc(1, sizeof(process));
    proc->command = calloc(strlen(command)+1, sizeof(char));
    proc->command = strdup(command);
    proc->pid = pid;
    return proc;
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

    if (!strcmp(process_args, "ps")) {
        vector_push_back(his_vec, arg);
        if (h) write_vec_to_file(his_vec, file_path);
        run_ps();
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
    char *potential_file_p = find_path(argv); 
    int dir_op = check_direction(argv, potential_file_p);
    if (dir_op) {
        if (h) write_vec_to_file(his_vec, file_path);
        if (dir_op == 1) { // OUTPUT >
            run_OUTPUT(argv, potential_file_p, dir_op);
        } else if (dir_op == 2) { // APPEND >>
            run_OUTPUT(argv, potential_file_p, dir_op);
        } else { // INPUT <
            run_INPUT(argv, potential_file_p);
        }
        return;
    }
    if (h) write_vec_to_file(his_vec, file_path);
//  signal command
    pid_t process_pid;
    if (!strcmp(argv[0], "kill")) {
        if (argv[1]) {
            process_pid = (pid_t) atoi(argv[1]);
        } else {
            print_invalid_command(process_args);
            exit(1);
        }
        kill_process(process_pid, 1, process_args);
        return;
    }
    if (!strcmp(argv[0], "stop")) {
        if (argv[1]) {
            process_pid = (pid_t) atoi(argv[1]);
        } else {
            print_invalid_command(process_args);
            exit(1);
        }
        kill_process(process_pid, 2, process_args);
        return;
    }
    if (!strcmp(argv[0], "cont")) {
        if (argv[1]) {
            process_pid = (pid_t) atoi(argv[1]);
        } else {
            print_invalid_command(process_args);
            exit(1);
        }
        kill_process(process_pid, 3, process_args);
        return;
    }
    run(argv[0], argv, process_args);
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
        run(process2_argv[0], process2_argv, process2);
        return;
    } 
// others || cd
    else if (!strcmp(process2_argv[0], "cd")) {
        run(process1_argv[0], process1_argv, process1);
        cd(process2_argv[1]);
        return;
    } 
// others || others
    else {
        run(process1_argv[0], process1_argv, process1);
        run(process2_argv[0], process2_argv, process2);
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
        if (!scd1) run(process2_argv[0], process2_argv, process2);
        return;
    } 
// others || cd
    else if (!strcmp(process2_argv[0], "cd")) {
        run(process1_argv[0], process1_argv, process1);
        if (command_failed) cd(process2_argv[1]);
        return;
    } 
// others || others
    else {
        run(process1_argv[0], process1_argv, process1);
        if (command_failed) run(process2_argv[0], process2_argv, process2);
        return;
    }
}

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
        if (scd1) run(process2_argv[0], process2_argv, process2);
        return;
    } 
// others && cd
    else if (!strcmp(process2_argv[0], "cd")) {
        run(process1_argv[0], process1_argv, process1);
        if (!command_failed) cd(process2_argv[1]);
        return;
    } 
// others  && others
    else {
        run(process1_argv[0], process1_argv, process1);
        if (!command_failed) run(process2_argv[0], process2_argv, process2);
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

void run_ps() {
    print_process_info_header();
    for (size_t i = 0; i < vector_size(processes_vec); i++) {
        process *proc = (process*) vector_get(processes_vec, i);
        if (kill(proc->pid, 0) != -1) {
            process_info *proc_info = load_info(proc->command, proc->pid);
            print_process_info(proc_info);
        }
    }
}

// failed: 0,  success: 1
int run(char* a, char** argv, char* command) {
    int index = 0;
    int is_background = check_background(argv, &index);
    if (is_background) {
        argv[index] = 0;
        fflush(stdin);
        fflush(stdout);
        pid_t child = fork();
        signal(SIGCHLD, child_handler);
        if (child == -1) {
            print_fork_failed();
            command_failed = 1;
            exit(1);
        }
        if (!child) {
            if (setpgid(child, child) == -1) {
                print_setpgid_failed();
                command_failed = 1;
                exit(1);
            }
            print_command_executed(getpid());
            execvp(a, argv);
            print_exec_failed(a);
            exit(1);
        } else {
            command_failed = 0;
            int s;
            // int ws = waitpid(child, &s, 0);
            // if (ws == -1) print_wait_failed();
            // if (WIFEXITED(s) && WEXITSTATUS(s) != 0) {
            //     // print_exec_failed(a);
            //     command_failed = 1;
            //     return 0;
            // }
            process *p = add_process(command, child);
            vector_push_back(processes_vec, p);
            waitpid(child, &s, WNOHANG);
            return 0;
        }



    } else {
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
    }
    // fflush(stdin);
    // fflush(stdout);
    // pid_t child = fork();
    // if (child == -1) {
    //     print_fork_failed();
    //     command_failed = 1;
    //     return 0;
    // }
    // if (!child) {
    //     print_command_executed(getpid());
    //     execvp(a, argv);
    //     print_exec_failed(a);
    //     exit(1);
    // } else {
    //     command_failed = 0;
    //     int s;
    //     int ws = waitpid(child, &s, 0);
    //     if (ws == -1) print_wait_failed();
    //     if (WIFEXITED(s) && WEXITSTATUS(s) != 0) {
    //         // print_exec_failed(a);
    //         command_failed = 1;
    //         return 0;
    //     }
    // }
    return 1;
}

int check_background(char **argv, int *index) {
    char **tmp = argv;
    int i = 0;
    while (*tmp) {
        if (!*(tmp+1)) {
            if (!strcmp(*tmp+strlen(*tmp)-1, "&")) {
                *index = i;
                return 1;
            } else {
                return 0;
            }
        }
        tmp++;
        i++;
    }
    
    return 0;
}

void child_handler(int sig) {
    int s;
    while (waitpid(-1, &s, WNOHANG) > 0) {}
}

// typedef struct process_info {
//     int pid;//
//     long int nthreads;
//     unsigned long int vsize;
//     char state;
//     char *start_str;
//     char *time_str;
//     char *command;
// } process_info;
process_info *load_info(char* command, pid_t pid) {
    process_info *proc_info = calloc(1, sizeof(process_info));
    proc_info->pid = (int) pid;
    // printf("pid is %d\n", proc_info->pid);
    proc_info->command = calloc(strlen(command)+1, sizeof(char));
    proc_info->command = strdup(command);
    char *proc_p = calloc(30, sizeof(char));
    strcpy(proc_p, "/proc/");
    int p = (int) pid;
    char *pid_s;
    asprintf(&pid_s, "%d", p);
    strcat(proc_p, pid_s);
    strcat(proc_p, "/status");

    FILE *fd = fopen(proc_p, "r");
    free(proc_p);
    if (fd) {
        char *buffer = NULL;
        size_t buffer_size = 0;
        while (getline(&buffer, &buffer_size, fd) > 0) {
            // state
            if (!strncmp(buffer, "State:", 6)) proc_info->state = *(buffer+7);
            // vsize
            if (!strncmp(buffer, "VmSize:", 7)) proc_info->vsize = strtol(strdup(buffer+8), NULL, 10);
            // nthreads
            if (!strncmp(buffer, "Threads:", 8)) proc_info->nthreads = strtol(strdup(buffer+9), NULL, 10);
        }
        fclose(fd);
    } else {
        print_script_file_error();
        exit(1);
    }

    char *proc_p_s = "/proc/stat";
    FILE *fd_s = fopen(proc_p_s, "r");
    unsigned long long btime;
    if (fd_s) {
        char *buffer = NULL;
        size_t buffer_size = 0;
        while (getline(&buffer, &buffer_size, fd_s) > 0) {
            // btime
            if (!strncmp(buffer, "btime ", 6)) {
                btime = strtoul(buffer+6, NULL, 10);
                break;
            }
            }
        fclose(fd_s);
    } else {
        print_script_file_error();
        exit(1);
    }

    char *proc_p_stat = calloc(30, sizeof(char));
    strcpy(proc_p_stat, "/proc/");
    strcat(proc_p_stat, pid_s);
    strcat(proc_p_stat, "/stat");
    FILE *fd_stat = fopen(proc_p_stat, "r");
    free(proc_p_stat);
    unsigned long utime, stime;
    unsigned long long starttime;
    if (fd_stat) {
        char *buffer = NULL;
        size_t buffer_size = 0;
        getline(&buffer, &buffer_size, fd_stat);
        char** stat = parse_string(buffer, " ");
        utime = strtoul(stat[13], NULL, 10);
        stime = strtoul(stat[14], NULL, 10);
        starttime = strtoull(stat[21], NULL, 10);

        fclose(fd_stat);
    } else {
        print_script_file_error();
        exit(1);
    }

// start_str
    time_t START_time = (time_t) (btime + starttime/sysconf(_SC_CLK_TCK));
    struct tm *tm_info = localtime(&START_time);
    char *buffer_start = calloc(30, sizeof(char));
    time_struct_to_string(buffer_start, 30, tm_info);
    proc_info->start_str = calloc(strlen(buffer_start)+1, sizeof(char));
    proc_info->start_str = strdup(buffer_start);

// time_str
    size_t TIME_time = (size_t) ((utime + stime)/sysconf(_SC_CLK_TCK));
    char *buffer_time = calloc(30, sizeof(char));
    size_t minutes = TIME_time / 60;
    size_t seconds = TIME_time % 60;
    execution_time_to_string(buffer_time, 30, minutes, seconds);
    proc_info->time_str = calloc(strlen(buffer_time)+1, sizeof(char));
    proc_info->time_str = strdup(buffer_time);

    return proc_info;
}

int check_direction(char **argv, char *path) {
    char **tmp = argv;
    while (*tmp) {
        if (!strcmp(*tmp, ">")) {
            path = *(tmp + 1);
            *tmp = 0;
            return 1;
        }
        else if (!strcmp(*tmp, ">>")) {
            path = *(tmp + 1);
            *tmp = 0;
            return 2;
        }
        else if (!strcmp(*tmp, "<")) {
            path = *(tmp + 1);
            *tmp = 0;
            return 3;
        }
        tmp++;
    }
    return 0;
}

char* find_path(char** argv) {
    char **tmp = argv;
    while (*tmp) {
        if (!*(tmp+1)) {
            return *tmp;
        }
        tmp++;
    }
    return NULL;
}

// reference: http://www.microhowto.info/howto/capture_the_output_of_a_child_process_in_c.html
int run_OUTPUT(char** argv, char* path, int mode) {
    int filedes[2];
    if (pipe(filedes) == -1) {
        print_redirection_file_error();
        exit(1);
    }

    // open file
    FILE *fd;
    if (mode == 1) {
        fd = fopen(path, "w+");
    } else if (mode == 2) {
        fd = fopen(path, "a+");
    }

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

        while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        close(filedes[1]);
        close(filedes[0]);
        execvp(argv[0], argv);
        print_exec_failed(argv[0]);
        exit(1);
    } else {
        close(filedes[1]);
        char buffer[4096];
        memset(buffer, 0, 4096);
        while (1) {
            ssize_t count = read(filedes[0], buffer, sizeof(buffer));
            if (count == -1) {
                if (errno == EINTR) {
                    continue;
                } else {
                    perror("read");
                    exit(1);
                }
            } else if (count == 0) {
                break;
            } else {
                fprintf(fd, "%s", buffer);
                fclose(fd);
            }
        }
        close(filedes[0]);

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

int run_INPUT(char** argv, char* path) {
    int filedes[2];
    if (pipe(filedes) == -1) { // entrance to the pipe is written to filedes[1] and the exit to filedes[0]
        print_redirection_file_error();
        exit(1);
    }

    // open file
    FILE *fd = fopen(path, "r");

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
        close(0);
        // while ((dup2(filedes[0], STDIN_FILENO) == -1) && (errno == EINTR)) {}
        dup(filedes[0]);
        close(filedes[0]);
        close(filedes[1]);

        execvp(argv[0], argv);
        print_exec_failed(argv[0]);
        exit(1);
    } else {
        close(filedes[0]);
        char *buffer = NULL;
        size_t buffer_size = 0;
        while (getline(&buffer, &buffer_size, fd) != -1) {
            write(filedes[1], buffer, strlen(buffer));
        }

        close(filedes[1]);

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

void kill_process(pid_t pid, int mode, char* command) {
    int s = 10; // if not changed, pid not found
    for (size_t i = 0; i < vector_size(processes_vec); i++) {
        process *proc = (process*) vector_get(processes_vec, i);
        if (proc->pid == pid) {
            if (mode == 1) {
                s = kill(pid, SIGKILL);
                print_killed_process(pid, command);
            } else if (mode == 2) {
                s = kill(pid, SIGSTOP);
                print_stopped_process(pid, command);
            } else {
                s = kill(pid, SIGCONT);
                print_continued_process(pid, command);
            }
        }
    }
    if (s == 10) {
        print_no_process_found((int) pid);
    }
}


