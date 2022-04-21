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

static int serverSocket;
static char *server_dir;
static int epfd;
static dictionary *clients;
static vector *file_vec;


int main(int argc, char **argv) {
    // good luck!
    if (argc != 2) {
        print_server_usage();
        exit(1);
    }
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
    struct epoll_event *evs;
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
        for (int i = 0; i < num_tasks; i++) {
            if (evs[i].data.fd == serverSocket) {
                int newfd = accept(serverSocket, (struct sockaddr *)&their_addr, &sin_size);
                if (newfd == -1) {
                    perror("accept");
                    exit(1);
                }
                struct epoll_event new_ev;
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
        }
    } else if (client->state == 2){
        //done
        epoll_ctl(epfd, EPOLL_CTL_DEL, client->fd, NULL);
        free(client);
        dictionary_remove(clients, &client->fd);
        shutdown(client->fd, SHUT_RDWR);
        close(client->fd);
    }
}

void parse_header(client_info *client) {
    client->state = 1;
    struct epoll_event e;
    e.events = EPOLLOUT;
    e.data.fd = client->fd;
    epoll_ctl(epfd, EPOLL_CTL_MOD, client->fd, &e);

    char *request_method = calloc(30, sizeof(char));
    // read_all_from_socket(client->fd, request_method, 30);
    recv(client->fd, request_method, 30, 0);

    if (!strncmp(request_method, "LIST", 4)) {
        client->v = LIST;
    } else if (!strncmp(request_method, "GET", 3)) {
        client->v = GET;
        strcpy(client->filename, request_method + 4);
    } else if (!strncmp(request_method, "DELETE", 6)) {
        client->v = DELETE;
        strcpy(client->filename, request_method + 7);
    } else if (!strncmp(request_method, "PUT", 3)) {
        client->v = PUT;
        strcpy(client->filename, request_method + 4);
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
    if (read_b != sizeof(size_t)) {
        perror("read");
    }
    
    FILE *receiver_fd;
    char file_path[100];
    sprintf(file_path, "%s/%s", server_dir, client->filename);
    receiver_fd = fopen(client->filename, "w+");
    if (!receiver_fd) {
        perror("fopen");
    }

    size_t total_read = 0;
    size_t bytes_read;
    size_t should_read;
    while (total_read < message_size + 5) {
        should_read = fmin(PACKET_SIZE, message_size + 5 - total_read);
        char buffer[PACKET_SIZE+1];
        bytes_read = read_all_from_socket(client->fd, buffer, should_read);
        fwrite(buffer, bytes_read, 1, receiver_fd);
        total_read += bytes_read;
        if (bytes_read == 0) break;
    }

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

    size_t write_ok_b = write_all_to_socket(client->fd, "OK\n", 3);
    if (write_ok_b != 3) {
        perror("write");
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
}

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
