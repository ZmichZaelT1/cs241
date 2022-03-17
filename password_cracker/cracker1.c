/**
 * password_cracker
 * CS 241 - Spring 2022
 */
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "./includes/queue.h"
#include <string.h>
#include <stdio.h>
#include <crypt.h>
#include <math.h>

static queue *task_q = NULL;
static int queue_size = 0;
static int success = 0;
static int failed = 0; 
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; 

void *cracker(void *i) {
    char *curr = NULL;
    char user[20], hash[20], known[20];
    while ((curr = queue_pull(task_q)) != NULL) {
        sscanf(curr, "%s %s %s", user, hash, known);
        v1_print_thread_start((long) i, user);
        double start = getThreadCPUTime();
        int prefixLength = getPrefixLength(known);
        int unknownLength = strlen(known) - prefixLength;

        struct crypt_data cdata;
        cdata.initialized = 0;
        char *unknown = known + prefixLength;
        setStringPosition(unknown, 0);
        int hashCount = 0;
        int result = 1;
        for (int i = 0; i < pow(26, unknownLength); i++) {
            char *guess = crypt_r(known, "xx", &cdata);
            hashCount++;
            if (!strcmp (guess, hash)) {
                pthread_mutex_lock(&mut);
                success++;
                pthread_mutex_unlock(&mut);
                result = 0;
                break;
            }
            incrementString(known);
        }
        v1_print_thread_result((long) i, user, known, hashCount, getThreadCPUTime() - start, result);
    }
    queue_push(task_q, NULL);
    return NULL;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads

    task_q = queue_create(-1);
    char *buf = NULL;
    size_t buf_size = 0;

    while (getline(&buf, &buf_size, stdin) != -1) {
        queue_push(task_q, strdup(buf));
        queue_size++;
    }
    queue_push(task_q, NULL);

    pthread_t pid[thread_count];
    for (size_t i = 0; i < thread_count; i++) {
        pthread_create(&pid[i], NULL, cracker, (void*) i+1);
    }
    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(pid[i], NULL);
    }

    v1_print_summary(success, failed);
    pthread_mutex_destroy(&mut);
    queue_destroy(task_q);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
