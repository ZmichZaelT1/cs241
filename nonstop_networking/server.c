/**
 * nonstop_networking
 * CS 241 - Spring 2022
 */
#include "format.h"
#include "common.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <math.h>
#include <signal.h>
#include "./includes/dictionary.h"
#include "./includes/vector.h"


#define MAXEVENTS 100
#define PACKET_SIZE 1000

typedef struct client_info_{
    int state; //0: need to parse header, 1: parsed header, 2: done
    int fd;
    verb v;
    char filename[50];
} client_info;

void *get_in_addr(struct sockaddr*);
void establishConnection(char*);
void process_client(int);
void parse_header(client_info*);
void run_LIST(client_info*);
void run_PUT(client_info*);
void run_GET(client_info*);
void run_DELETE(client_info*);
void read_until_new_line(int, char*);
void sig_handler();
void sigpipe_handler();

static int serverSocket;
static char *server_dir;
static int epfd;
static dictionary *clients;
static vector *file_vec;
static struct epoll_event *evs;


int main(int argc, char **argv) {
    // good luck!
    if (argc != 2) {
        print_server_usage();
        exit(1);
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_handler;
    sigaction(SIGINT, &sa, NULL);

    signal(SIGPIPE, sigpipe_handler);

    char *port = argv[1];
    establishConnection(port);
    char dir[] = "XXXXXX";
    server_dir = mkdtemp(dir);
    print_temp_directory(server_dir);

    clients = int_to_shallow_dictionary_create();
    file_vec = string_vector_create();

    epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create");
        exit(1);
    }
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    // struct epoll_event *evs;
    ev.events = EPOLLIN;
    ev.data.fd = serverSocket;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, serverSocket, &ev) == -1) {
        perror("epoll_ctl");
        exit(1);
    }
    evs = calloc(MAXEVENTS, sizeof(ev));

    struct sockaddr_storage their_addr;
    while(1) {
        socklen_t sin_size = sizeof(their_addr);
        int num_tasks;
        num_tasks = epoll_wait(epfd, evs, MAXEVENTS, -1);
        fprintf(stderr, "epoll_wait return: %d\n", num_tasks);
        for (int i = 0; i < num_tasks; i++) {
            if (evs[i].data.fd == serverSocket) {
                int newfd = accept(serverSocket, (struct sockaddr *)&their_addr, &sin_size);
                fprintf(stderr, "(fd=%d) Accepted new connection\n", newfd);
                if (newfd == -1) {
                    perror("accept");
                    exit(1);
                }
                struct epoll_event new_ev;
                memset(&new_ev, 0, sizeof(new_ev));
                new_ev.events = EPOLLIN;
                new_ev.data.fd = newfd;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, &new_ev) == -1) {
                    perror("epoll_ctl");
                    exit(1);
                }
                client_info *client = calloc(1, sizeof(client_info)); 
                dictionary_set(clients, &newfd, client);
            } else {
                process_client(evs[i].data.fd);
            }
        }
    }
}

void process_client(int fd) {
    client_info *client = dictionary_get(clients, &fd);
    if (client->state == 0) {
        client->fd = fd;
        parse_header(client);
    } else if (client->state == 1) {
        if (client->v == LIST) {
            run_LIST(client);
        } else if (client->v == PUT) {
            run_PUT(client);
        } else if (client->v == GET) {
            run_GET(client);
        } else if (client->v == DELETE) {
            run_DELETE(client);
        } else {
            char *error = "ERROR\n";
            write_all_to_socket(client->fd, error, strlen(error));
            char *error_msg = "Bad request";
            write_all_to_socket(client->fd, error_msg, strlen(error_msg));
        }
    } else if (client->state == 2){
        //done
        fprintf(stderr, "(fd=%d) finished, shutting it done\n", client->fd);
        epoll_ctl(epfd, EPOLL_CTL_DEL, client->fd, NULL);
        dictionary_remove(clients, &client->fd);
        shutdown(client->fd, SHUT_RDWR);
        close(client->fd);
        free(client);
    }
}

void parse_header(client_info *client) {
    client->state = 1;
    struct epoll_event e;
    memset(&e, 0, sizeof(e));
    e.events = EPOLLOUT;
    e.data.fd = client->fd;
    epoll_ctl(epfd, EPOLL_CTL_MOD, client->fd, &e);

    char *request_method = calloc(30, sizeof(char));
    // read_all_from_socket(client->fd, request_method, 30);
    // recv(client->fd, request_method, 30, 0);
    read_until_new_line(client->fd, request_method);

    if (!strncmp(request_method, "LIST", 4)) {
        client->v = LIST;
    } else if (!strncmp(request_method, "GET", 3)) {
        client->v = GET;
        strcpy(client->filename, request_method + 4);
        client->filename[strlen(client->filename)-1] = 0;
    } else if (!strncmp(request_method, "DELETE", 6)) {
        client->v = DELETE;
        strcpy(client->filename, request_method + 7);
        client->filename[strlen(client->filename)-1] = 0;
    } else if (!strncmp(request_method, "PUT", 3)) {
        client->v = PUT;
        strcpy(client->filename, request_method + 4);
        client->filename[strlen(client->filename)-1] = 0;
        fprintf(stderr, "(fd=%d) PUT requested for '%s'\n", client->fd, client->filename);
        ///
    } else {
        client->state = -1;
        print_invalid_response();
    }
    free(request_method);
}

void run_LIST(client_info *client) {
    size_t message_size = 0;
    VECTOR_FOR_EACH(file_vec, file, {message_size += strlen(file) + 1;});
    if (message_size > 0) {
        message_size--;
    }
    size_t write_ok_b = write_all_to_socket(client->fd, "OK\n", 3);
    if (write_ok_b != 3) {
        perror("write");
    }

    size_t write_size_b = write_all_to_socket(client->fd, (char*)&message_size, sizeof(size_t));
    if (write_size_b != sizeof(size_t)) {
        perror("write");
    }

    VECTOR_FOR_EACH(file_vec, file, {
        write_all_to_socket(client->fd, file, strlen(file));
        if (_it != _iend - 1) {
            write_all_to_socket(client->fd, "\n", 1);
        }
    });
    client->state = 2;
}

void run_PUT(client_info *client) {
    size_t message_size;
    size_t read_b = read_all_from_socket(client->fd, (char*)&message_size, sizeof(size_t));
    fprintf(stderr, "(fd=%d) file size %zu bytes\n", client->fd, message_size);
    if (read_b != sizeof(size_t)) {
        perror("read");
    }
    
    FILE *receiver_fd;
    char file_path[100];
    sprintf(file_path, "%s/%s", server_dir, client->filename);
    receiver_fd = fopen(file_path, "w+");
    if (!receiver_fd) {
        perror("fopen");
    }

    size_t total_read = 0;
    size_t bytes_read;
    size_t should_read;
    fprintf(stderr, "(fd=%d) Reading binary data from request\n", client->fd);
    while (total_read < message_size + 5) {
        should_read = fmin(PACKET_SIZE, message_size + 5 - total_read);
        char buffer[PACKET_SIZE+1];
        bytes_read = read_all_from_socket(client->fd, buffer, should_read);
        fwrite(buffer, bytes_read, 1, receiver_fd);
        total_read += bytes_read;
        if (bytes_read == 0) break;
    }
    fclose(receiver_fd);

    if (total_read != message_size) {
        remove(file_path);
        client->state = -1;
        char *error = "ERROR\n";
        write_all_to_socket(client->fd, error, strlen(error));
        char *error_msg = "Bad file size";
        write_all_to_socket(client->fd, error_msg, strlen(error_msg));
        return;
    }

    int file_exist = 0;
    VECTOR_FOR_EACH(file_vec, file, {
        if (!strcmp(file, client->filename)) {
            file_exist = 1;
            break;
        }
    });
    
    if (!file_exist) {
        vector_push_back(file_vec, client->filename);
    }

    fprintf(stderr, "(fd=%d) Writing header for response OK\n", client->fd);
    // size_t write_ok_b = write_all_to_socket(client->fd, "OK\n", 3);
    // if (write_ok_b != 3) {
    //     perror("write");
    // }
    write(client->fd, "OK\n", 3);

    fprintf(stderr, "(fd=%d) Finished writing response\n", client->fd);
    client->state = 2;
}

void run_GET(client_info *client) {
    FILE *request_file;
    char file_path[100];
    sprintf(file_path, "%s/%s", server_dir, client->filename);
    request_file = fopen(file_path, "r");
    if (!request_file) {
        perror("fopen");
        char *error = "ERROR\n";
        write_all_to_socket(client->fd, error, strlen(error));
        char *error_msg = "No such file";
        write_all_to_socket(client->fd, error_msg, strlen(error_msg));
        client->state = 2;
        return;
    }

    size_t write_ok_b = write_all_to_socket(client->fd, "OK\n", 3);
    if (write_ok_b != 3) {
        perror("write");
    }

    fseek(request_file, 0L, SEEK_END);
    size_t message_size = ftell(request_file);
    rewind(request_file);

    write_all_to_socket(client->fd, (char*)&message_size, sizeof(message_size));

    size_t bytes_sent = 0;
    size_t should_send;
    while(bytes_sent < message_size) {
        should_send = fmin(PACKET_SIZE, message_size - bytes_sent);
        char buffer[should_send + 1];
        fread(buffer, 1, should_send, request_file);
        write_all_to_socket(client->fd, buffer, should_send);
        bytes_sent += should_send;
    }
    fclose(request_file);
    client->state = 2;
}

void run_DELETE(client_info *client) {
    char file_path[100];
    sprintf(file_path, "%s/%s", server_dir, client->filename);
    if (remove(file_path) == -1) {
        char *error = "ERROR\n";
        write_all_to_socket(client->fd, error, strlen(error));
        char *error_msg = "No such file";
        write_all_to_socket(client->fd, error_msg, strlen(error_msg));
        client->state = 2;
        return;
    }

    size_t write_ok_b = write_all_to_socket(client->fd, "OK\n", 3);
    if (write_ok_b != 3) {
        perror("write");
    }

    size_t i = 0;
    VECTOR_FOR_EACH(file_vec, file, {
        if (!strcmp(file, client->filename)) {
            vector_erase(file_vec, i);
            break;
        }
        i++;
    });
    client->state = 2;
}

void read_until_new_line(int socket, char *buffer) {
    size_t bytes_read = 0;
    while(1) {
        size_t s = read(socket, buffer + bytes_read, 1);
        if (buffer[bytes_read] == '\n') break;
        bytes_read += s;
        if (s == 0) break;
    }
}

void establishConnection(char *port) {
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rv = getaddrinfo(NULL, port, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    serverSocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (serverSocket == -1) {
        perror("server: socket");
        exit(1);
    }
    
    int one = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    int ok = bind(serverSocket, servinfo->ai_addr, servinfo->ai_addrlen);
    if (ok == -1) {
        perror("Server: Bind");
        exit(1);
    }
    LOG("Connected!");

	if (listen(serverSocket, 1) == -1) {
		perror("listen");
		exit(1);
	}
    freeaddrinfo(servinfo);
}

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sig_handler() {
    vector *tmp = dictionary_values(clients);
    printf("is empty: %d", vector_empty(tmp));
    VECTOR_FOR_EACH(tmp, t, {free(t);});
    vector_destroy(tmp);
    dictionary_destroy(clients);
    char file_path[100];
    VECTOR_FOR_EACH(file_vec, file, {
        sprintf(file_path, "%s/%s", server_dir, file);
        remove(file_path);
    });
    vector_destroy(file_vec);
    remove(server_dir);
    close(epfd);
    free(evs);
    exit(1);
}

void sigpipe_handler() {}

// TODO: detects too much/little data 
// Tests that we can PUT large files with 2 clients (