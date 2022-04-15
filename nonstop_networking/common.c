/**
 * nonstop_networking
 * CS 241 - Spring 2022
 */
#include "common.h"



#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>




ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    size_t bytes_written = 0;
    while (bytes_written < count) {
        ssize_t bytes_read = read(socket, buffer + bytes_written, count - bytes_written);
        if (bytes_read == 0) {
            break;
        }
        if (bytes_read == -1 && errno == EINTR) {
            continue;
        }
        if (bytes_read == -1) {
            return -1;
        }
        bytes_written += bytes_read;
    }
    return bytes_written;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    size_t bytes_read = 0;
    while (bytes_read < count) {
        ssize_t bytes_written = write(socket, buffer + bytes_read, count - bytes_read);
        if (bytes_written == 0) {
            break;
        } 
        if (bytes_written == -1 && errno == EINTR) {
            continue;
        } 
        if (bytes_written == -1) {
            return -1;
        }
        bytes_read += bytes_written;
    }
    return bytes_read;
}
