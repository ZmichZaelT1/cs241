/**
 * teaching_threads
 * CS 241 - Spring 2022
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */

typedef struct data {
    int *list;
    size_t list_len;
    reducer reduce_func;
    int base_case;
}data;

int **split_list(int*, size_t, size_t);
void *partial_reduce(void*);
/* You should create a start routine for your threads. */

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    if (num_threads >= list_len) {
        return reduce(list, list_len, reduce_func, base_case);
    }

// split the job
    int **splited_list = split_list(list, list_len, num_threads);
    data* data_list[num_threads];
    int each_size = list_len / num_threads;
    int extra_size = list_len % num_threads;
    for (int i = 0; i < (int) num_threads; i++) {
        data_list[i] = calloc(1, sizeof(data));
        if (i == (int) num_threads - 1) {
            data_list[i]->list_len = each_size + extra_size;
        } else {
            data_list[i]->list_len = each_size;
        }
        data_list[i]->list = splited_list[i];
        data_list[i]->reduce_func = reduce_func;
        data_list[i]->base_case = base_case;
    }

// Assign to threads
    pthread_t threads[num_threads];
    for (int i = 0; i < (int) num_threads; i++) {
        pthread_create(threads + i, 0, partial_reduce, (void*) data_list[i]);
    }

// wait for threads
    int sum_list[num_threads];
    for (int i = 0; i < (int) num_threads; i++) {
        void *thread_ret;
        pthread_join(threads[i], &thread_ret);
        sum_list[i] = *(int*) thread_ret;
    }

    return reduce(sum_list, num_threads, reduce_func, base_case);
}

void *partial_reduce(void *input) {
    data *list = (data*) input;
    int result = list->base_case;

    for (size_t i = 0; i < list->list_len; ++i) {
        result = list->reduce_func(result, list->list[i]);
    }
    int *res = calloc(1, sizeof(int));
    *res = result;
    return res;
}

int **split_list(int *list, size_t list_len, size_t num_threads) {
    int **ret = calloc(num_threads, sizeof(int*));
    int each_size = list_len / num_threads;
    int extra_size = list_len % num_threads;

    for (int i = 0; i < (int) num_threads; i++) {
        if (i == (int) num_threads - 1) {
            ret[i] = calloc(extra_size + each_size, sizeof(int));
            memcpy(ret[i], list+i*each_size, (extra_size + each_size) * sizeof(int));
            break;
        }
        ret[i] = calloc(each_size, sizeof(int));
        memcpy(ret[i], list+i*each_size, each_size * sizeof(int));
    }
    return ret;
}
