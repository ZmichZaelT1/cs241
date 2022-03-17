/**
 * password_cracker
 * CS 241 - Spring 2022
 */
#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <crypt.h>

static int success = 0;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t start_b;
pthread_barrier_t end_b;

static int total_hashCount = 0;
// static char *result = NULL;
static int eof = 0;
static int thread_number = 0;
static char user[20];
static char hash[20];
static char known[20];
static char *result;


void *cracker(void *i) {
    struct crypt_data cdata;
    cdata.initialized = 0;
    char *known_cpy = NULL;
    while(1){
        pthread_barrier_wait(&start_b);
        if(eof) break;
        int hashCount = 0;
        long start_index = 0;
        long count = 0;
        int prefixLength = getPrefixLength(known);
        int unknown_letter_count = strlen(known) - prefixLength;
        getSubrange(unknown_letter_count, thread_number, (long) i, &start_index, &count);

        known_cpy = calloc(strlen(known) + 1 + prefixLength, sizeof(char));
        strcpy(known_cpy, known);
        char *unknown = known_cpy + prefixLength;
        setStringPosition(unknown, start_index);
        v2_print_thread_start((long) i, user, start_index, known_cpy);

        for (int j = 0; j < count; j++){
            char *guess = crypt_r(known_cpy, "xx", &cdata);
            hashCount++;

            if(!strcmp(guess, hash)){
                pthread_mutex_lock(&mut);
                result = calloc(1, strlen(known_cpy) + 1);
                strcpy(result, known_cpy);
                success = 1;
                v2_print_thread_result((long) i, hashCount, 0);
                total_hashCount = total_hashCount + hashCount;
                pthread_mutex_unlock(&mut);
                break;
            }

            if(success){
                pthread_mutex_lock(&mut);
                v2_print_thread_result((long) i, hashCount, 1);
                total_hashCount = total_hashCount + hashCount;
                pthread_mutex_unlock(&mut);
                break;
            }

            incrementString(known_cpy);
        }
        free(known_cpy);

        if(!success){
            pthread_mutex_lock(&mut);
            v2_print_thread_result((long) i, hashCount, 2);
            total_hashCount = total_hashCount + hashCount;
            pthread_mutex_unlock(&mut);
        }
        pthread_barrier_wait(&end_b);
    }
    return NULL;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    thread_number = thread_count;

    pthread_barrier_init(&start_b, NULL, thread_count + 1);
    pthread_barrier_init(&end_b, NULL, thread_count + 1);

    pthread_t pid[thread_count];
    for (size_t i = 0; i < thread_count; i++) {
        pthread_create(&pid[i], NULL, cracker, (void*) i+1);
    }

    char *buf = NULL;
    size_t buf_size = 0;
    while (getline(&buf, &buf_size, stdin) != -1) {
        sscanf(buf, "%s %s %s", user, hash, known);
        v2_print_start_user(user);

        pthread_barrier_wait(&start_b); // start cracker
        double time_start = getTime();
        double cpu_start = getCPUTime();

        pthread_barrier_wait(&end_b);
        v2_print_summary(user, result, total_hashCount, getTime() - time_start, getCPUTime() - cpu_start, !success);
        success = 0;
        total_hashCount = 0;
        free(result);
        result = NULL;
        // free(buf);
    }
    free(buf);
    eof = 1;
    pthread_barrier_wait(&start_b);

    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(pid[i], NULL);
    }
    pthread_mutex_destroy(&mut);
    pthread_barrier_destroy(&start_b);
    pthread_barrier_destroy(&end_b);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
