/**
 * critical_concurrency
 * CS 241 - Spring 2022
 */
#include "barrier.h"

// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
    int error;
    error = pthread_mutex_destroy(&barrier->mtx);
    if (!error) error = pthread_cond_destroy(&barrier->cv);
    return error;
}

int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    int error;
    error = pthread_mutex_init(&barrier->mtx, NULL);
    if (!error) error = pthread_cond_init(&barrier->cv, NULL);
    barrier->n_threads = num_threads;
    barrier->count = num_threads;
    barrier->times_used = 0;
    return error;
}

int barrier_wait(barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mtx);
    barrier->count--;
    if (barrier->count == 0) {
        barrier->times_used++;
        barrier->count = barrier->n_threads;

        pthread_cond_broadcast(&barrier->cv);
        pthread_mutex_unlock(&barrier->mtx);
        return 1;
    } else {
        unsigned int tmp = barrier->times_used;

        while (tmp == barrier->times_used) {
            pthread_cond_wait(&barrier->cv, &barrier->mtx);
        }
        pthread_mutex_unlock(&barrier->mtx);
        return 0;
    }
}

// reference: http://byronlai.com/jekyll/update/2015/12/26/barrier.html