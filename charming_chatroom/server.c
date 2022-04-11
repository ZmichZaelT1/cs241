/**
 * charming_chatroom
 * CS 241 - Spring 2022
 */
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

#include "utils.h"

#define MAX_CLIENTS 8

void *process_client(void *p);

static volatile int serverSocket;
static volatile int endSession;

static volatile int clientsCount;
static volatile int clients[MAX_CLIENTS];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Signal handler for SIGINT.
 * Used to set flag to end server.
 */
void close_server() {
    endSession = 1;
    // add any additional flags here you want.

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            if (shutdown(clients[i], SHUT_RDWR) != 0) {
                perror("shutdown");
            }
            if (close(clients[i]) != 0) {
                perror("close");
            }
        }
    }
    if (shutdown(serverSocket, SHUT_RDWR) != 0) {
        perror("shutdown");
    }
    if (close(serverSocket) != 0) {
        perror("close");
    }
}

/**
 * Cleanup function called in main after `run_server` exits.
 * Server ending clean up (such as shutting down clients) should be handled
 * here.
 */
void cleanup() {
    if (shutdown(serverSocket, SHUT_RDWR) != 0) {
        perror("shutdown():");
    }
    close(serverSocket);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            if (shutdown(clients[i], SHUT_RDWR) != 0) {
                perror("shutdown(): ");
            }
            close(clients[i]);
        }
    }
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


/**
 * Sets up a server connection.
 * Does not accept more than MAX_CLIENTS connections.  If more than MAX_CLIENTS
 * clients attempts to connects, simply shuts down
 * the new client and continues accepting.
 * Per client, a thread should be created and 'process_client' should handle
 * that client.
 * Makes use of 'endSession', 'clientsCount', 'client', and 'mutex'.
 *
 * port - port server will run on.
 *
 * If any networking call fails, the appropriate error is printed and the
 * function calls exit(1):
 *    - fprtinf to stderr for getaddrinfo
 *    - perror() for any other call
 */
void run_server(char *port) {
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rv = getaddrinfo(NULL, port, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
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
    puts("Connected!");

	if (listen(serverSocket, MAX_CLIENTS) == -1) {
		perror("listen");
		exit(1);
	}

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = -1;
    }

    pthread_t tid[MAX_CLIENTS];
    socklen_t sin_size;
    int new_fd;
    while (!endSession) {
        if (clientsCount > MAX_CLIENTS) {
            continue;
        }
        struct sockaddr_storage their_addr;
        sin_size = sizeof(their_addr);
        new_fd = accept(serverSocket, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            exit(1);
        }

        intptr_t p = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == -1) {
                clients[i] = new_fd;
                char str[INET6_ADDRSTRLEN];
                inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), str, sizeof(str));
                printf("server: got connection from %s\n", str);
                p = i;
                break;
            }
        }
        if (pthread_create(&tid[p], NULL, process_client, (void*)p)) {
            perror("thread: create");
            exit(1);
        }
        clientsCount++;
    }
    freeaddrinfo(servinfo);
}

/**
 * Broadcasts the message to all connected clients.
 *
 * message  - the message to send to all clients.
 * size     - length in bytes of message to send.
 */
void write_to_clients(const char *message, size_t size) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            ssize_t retval = write_message_size(size, clients[i]);
            if (retval > 0) {
                retval = write_all_to_socket(clients[i], message, size);
            }
            if (retval == -1) {
                perror("write(): ");
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

/**
 * Handles the reading to and writing from clients.
 *
 * p  - (void*)intptr_t index where clients[(intptr_t)p] is the file descriptor
 * for this client.
 *
 * Return value not used.
 */
void *process_client(void *p) {
    pthread_detach(pthread_self());
    intptr_t clientId = (intptr_t)p;
    ssize_t retval = 1;
    char *buffer = NULL;

    while (retval > 0 && endSession == 0) {
        retval = get_message_size(clients[clientId]);
        if (retval > 0) {
            buffer = calloc(1, retval);
            retval = read_all_from_socket(clients[clientId], buffer, retval);
        }
        if (retval > 0)
            write_to_clients(buffer, retval);

        free(buffer);
        buffer = NULL;
    }

    printf("User %d left\n", (int)clientId);
    close(clients[clientId]);

    pthread_mutex_lock(&mutex);
    clients[clientId] = -1;
    clientsCount--;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s <port>\n", argv[0]);
        return -1;
    }

    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = close_server;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    run_server(argv[1]);
    cleanup();
    pthread_exit(NULL);
}
