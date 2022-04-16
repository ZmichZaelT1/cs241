/**
 * nonstop_networking
 * CS 241 - Spring 2022
 */
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>

#include "common.h"

char **parse_args(int argc, char **argv);
verb check_args(char **args);
void *get_in_addr(struct sockaddr*);
void run_verb(char*, char*, char*);
void run_PUT(char*, char*);
void read_DELETE();
void read_LIST();
void read_PUT();
void read_GET();
void read_error(char*);
void establishConnection(char*, char*);
void closeConnection();

static volatile int sock_fd = 0;
static size_t packet_size = 1000;

int main(int argc, char **argv) {
    // Good luck!
    if (argc < 3) {
        print_client_usage();
        return 0;
    } 
    char ** args = parse_args(argc, argv);
    verb v = check_args(args);
    
    char *host = args[0];
    char *port = args[1];
    char *v_c = args[2];
    char *remote = args[3];
    char *local = args[4];

    establishConnection(host, port);

    if (v == GET) {
        run_verb(v_c, remote, local);
        LOG("GET!");
        read_GET(local);
    } else if (v == PUT) {
        run_verb(v_c, remote, local);
        LOG("PUT!");
        read_PUT();
    } else if (v == DELETE) {
        run_verb(v_c, remote, local);
        LOG("DELETE!");
        read_DELETE();
    } else if (v == LIST) {
        run_verb(v_c, remote, local);
        LOG("LIST!");
        read_LIST();
    }

    closeConnection();
    free(args);
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void read_PUT() {
    char *ok = "OK\n";
    printf("%s", ok);
    print_success();
}

void read_DELETE() {
    char *ok = "OK\n";
    char *buf = calloc(1, strlen(ok) + 1);
    ssize_t bytes_read = read_all_from_socket(sock_fd, buf, strlen(ok));
    if (bytes_read != (ssize_t)strlen(ok)) {
        print_connection_closed();
        exit(1);
    }

    if (!strcmp(buf, ok)) { // if OK
        printf("%s", ok);
        print_success();
    } else {
        read_error(buf);
    }
    free(buf);
}

void read_LIST() {
    char *ok = "OK\n";
    char *buf = calloc(1, strlen(ok) + 1);
    ssize_t bytes_read = read_all_from_socket(sock_fd, buf, strlen(ok));
    if (bytes_read != (ssize_t)strlen(ok)) {
        print_connection_closed();
        exit(1);
    }

    if (!strcmp(buf, ok)) { // if OK
        printf("%s", ok);
        size_t message_size;
        ssize_t bytes_read = read_all_from_socket(sock_fd, (char*)&message_size, sizeof(message_size));
        if (bytes_read != (ssize_t) sizeof(message_size)) {
            print_connection_closed();
            exit(1);
        }
        printf("%zu\n", message_size);
        // char *list_info = calloc(1, sizeof(char) * (message_size));

        char list_info[message_size + 5];
        memset(list_info, 0, message_size + 5);
        ssize_t info_bytes = read_all_from_socket(sock_fd, list_info, message_size + 4);
        if (info_bytes == 0 || info_bytes == -1) {
            print_connection_closed();
            exit(1);
        } else if (info_bytes < (ssize_t)message_size) {
            print_too_little_data();
            exit(1);
        } else if (info_bytes > (ssize_t) message_size) {
            print_received_too_much_data();
            exit(1);
        }
        printf("%s", list_info);

    } else { // if ERROR
        read_error(buf);
    }
    free(buf);
}

void read_GET(char *local) {
    char *ok = "OK\n";
    char *buf = calloc(1, strlen(ok) + 1);
    ssize_t bytes_read = read_all_from_socket(sock_fd, buf, strlen(ok));
    if (bytes_read != (ssize_t)strlen(ok)) {
        print_connection_closed();
        exit(1);
    }

    if (!strcmp(buf, ok)) { // if OK
        printf("%s", ok);
        size_t message_size;
        ssize_t bytes_read = read_all_from_socket(sock_fd, (char*)&message_size, sizeof(message_size));
        if (bytes_read != (ssize_t) sizeof(message_size)) {
            print_connection_closed();
            exit(1);
        }
        printf("%zu", message_size);
        
        FILE *fd;
        fd = fopen(local, "w+");
        if (!fd) {
            perror("open file");
            exit(1);
        }

        bytes_read = 0;
        size_t should_read;
        while (bytes_read < (ssize_t)message_size) {
            if (message_size - bytes_read > (size_t)packet_size) {
                should_read = packet_size;
            } else {
                should_read = message_size - bytes_read;
            }
            char buffer[should_read+5];
            size_t bytes_received = read_all_from_socket(sock_fd, buffer, should_read + 4);
            if (bytes_received == 0) break;
            // if (bytes_received <= should_read) {
            //     print_connection_closed();
            //     exit(1);
            // }
            fwrite(buffer, should_read, 1, fd);
            bytes_read += bytes_received;
        } 
        if (bytes_read < (ssize_t)message_size) {
            print_too_little_data();
            exit(1);
        } else if (bytes_read > (ssize_t) message_size) {
            print_received_too_much_data();
            exit(1);
        }
    } else { // if ERROR
        read_error(buf);
    }
    free(buf);
}

void read_error(char *buf) {
    char *ok = "OK\n";
    char *error = "ERROR\n";
    buf = realloc(buf, strlen(error) + 1);
    ssize_t bytes_read = read_all_from_socket(sock_fd, buf + strlen(ok), strlen(error) - strlen(ok));
    if (bytes_read != (ssize_t)(strlen(error) - strlen(ok))) {
        print_connection_closed();
        exit(1);
    }


    if (strcmp(buf, error)) {
        print_invalid_response();
    } else {
        printf("%s", buf);
        char *errorMessage = calloc(1, sizeof(char) * 100);
        ssize_t bytes_read = read_all_from_socket(sock_fd, errorMessage, sizeof(char) * 100);
        if (bytes_read != (ssize_t)strlen(errorMessage)) {
            print_connection_closed();
            exit(1);
        }
        print_error_message(errorMessage);
        free(errorMessage);
    }
}

void run_verb(char *v, char *remote, char *local) {
    if (!strcmp(v, "LIST")) {
        char *request = "LIST\n";
        ssize_t bytes_written = write_all_to_socket(sock_fd, request, strlen(request));
        if (bytes_written != (ssize_t)strlen(request)) {
            print_connection_closed();
            exit(1);
        }
    } else if (!strcmp(v, "PUT")) {
        run_PUT(remote, local);
    } else {
        char request[3 + strlen(v) + strlen(remote)];
        sprintf(request, "%s %s\n", v, remote);
        ssize_t bytes_written = write_all_to_socket(sock_fd, request, strlen(request));
        if (bytes_written != (ssize_t)strlen(request)) {
            print_connection_closed();
            exit(1);
        }
    }
}

void run_PUT(char *remote, char *local) {
    FILE *fd;
    fd = fopen(local, "r");
    if (!fd) {
        perror("file open");
        exit(1);
    }
    fseek(fd, 0L, SEEK_END);
    size_t fsize = ftell(fd);
    rewind(fd);
    
    char *put = "PUT ";
    size_t request_size = strlen(put)+strlen(remote)+1;
    char request[request_size];
    sprintf(request, "%s%s\n", put, remote);
    // printf("%s", request);
    ssize_t s1 = write_all_to_socket(sock_fd, request, strlen(request));
    ssize_t s2 = write_all_to_socket(sock_fd, (char*)&fsize, sizeof(fsize));

    if (s1+s2 != (ssize_t)request_size + (ssize_t)sizeof(fsize)) {
        print_connection_closed();
        exit(1);
    }

    size_t uploaded_size = 0;
    size_t should_write;
    while (uploaded_size < fsize) {
        if (fsize - uploaded_size > packet_size) {
            should_write = packet_size;
        } else {
            should_write = fsize - uploaded_size;
        }

        char buffer[should_write + 1];
        fread(buffer, 1, should_write, fd);
        ssize_t bytes_written = write_all_to_socket(sock_fd, buffer, should_write);
        if (bytes_written != (ssize_t)should_write) {
            print_connection_closed();
            exit(1);
        }
        uploaded_size += should_write;
    }
    fclose(fd);
}

void establishConnection(char *host, char *port) {
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int rv = getaddrinfo(host, port, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }

    sock_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sock_fd == -1) {
        perror("client: socket");
        exit(1);
    }

    int ok = connect(sock_fd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (ok == -1) {
        perror("Client: connect");
        exit(1);
    }

    // char str[INET6_ADDRSTRLEN];
	// inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *)servinfo->ai_addr),
	// 		str, sizeof str);
	// fprintf(stderr, "client: connecting to %s\n", str);
    LOG("connecting");
    freeaddrinfo(servinfo); 
}

void closeConnection() {
    if (shutdown(sock_fd, SHUT_RDWR) != 0) {
        perror("shutdown");
    }
    if (close(sock_fd) != 0) {
        perror("close");
    }
}

