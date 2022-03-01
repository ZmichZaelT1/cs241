/**
 * critical_concurrency
 * CS 241 - Spring 2022
 */
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"

int main(int argc, char **argv) {
    queue *test = queue_create(3);
    queue_push(test, (void*) 1);
    queue_pull(test);
    queue_pull(test);

    queue_destroy(test);
    return 0;
}
