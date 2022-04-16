/**
 * nonstop_networking
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

void *get_in_addr(struct sockaddr *sa);

static volatile int serverSocket;

int main(int argc, char **argv) {
    // good luck!
    char *port = argv[1];
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

	if (listen(serverSocket, 1) == -1) {
		perror("listen");
		exit(1);
	}



    struct sockaddr_storage their_addr;
    while (1) {
        socklen_t sin_size = sizeof(their_addr);
        int newfd = accept(serverSocket, (struct sockaddr *)&their_addr, &sin_size);

        char str[INET6_ADDRSTRLEN];
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), str, sizeof(str));
        printf("server: got connection from %s\n", str);
        sleep(1);
        // char *buffer = calloc(1, 50);
        // read(newfd, buffer, 49);
        // printf("request: %s\n", buffer);

        char *okk = "OK\n";
        write(newfd, okk, strlen(okk));

        size_t size = 10;
        write(newfd, (char*)&size, sizeof(size));

        char *random = "1234";
        write(newfd, random, strlen(random));
        shutdown(newfd, SHUT_RDWR);
        close(newfd);
    }
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
