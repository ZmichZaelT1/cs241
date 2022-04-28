/**
 * savvy_scheduler
 * CS 241 - Spring 2022
 */
#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_functions.h"

/**
 * The struct to hold the information about a given job
 */
typedef struct _job_info {
    int id;
    double start_time;
    double end_time;
    double run_time;
    double arrival_time;
    double left_time;
    double last_run;
    int priority;

    /* TODO: Add any other information and bookkeeping you need into this
     * struct. */
} job_info;

void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any additional set up code you may need here
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *j_a = ((job*)a)->metadata;
    job_info *j_b = ((job*)b)->metadata;
    if (j_a->arrival_time - j_b->arrival_time < 0) {
        return -1;
    } else if (j_a->arrival_time - j_b->arrival_time > 0) {
        return 1;
    }
    return 0;
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *j_a = ((job*)a)->metadata;
    job_info *j_b = ((job*)b)->metadata;
    if (j_a->priority - j_b->priority < 0) {
        return -1;
    } else if (j_a->priority - j_b->priority > 0){
        return 1;
    }
    return break_tie(a, b);
}

int comparer_psrtf(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *j_a = ((job*)a)->metadata;
    job_info *j_b = ((job*)b)->metadata;
    if (j_a->left_time - j_b->left_time < 0) {
        return -1;
    } else if (j_a->left_time - j_b->left_time > 0){
        return 1;
    }
    return break_tie(a, b);
}

int comparer_rr(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *j_a = ((job*)a)->metadata;
    job_info *j_b = ((job*)b)->metadata;
    if (j_a->last_run - j_b->last_run < 0) {
        return -1;
    } else if (j_a->last_run - j_b->last_run > 0){
        return 1;
    }
    return break_tie(a, b);
}

int comparer_sjf(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *j_a = ((job*)a)->metadata;
    job_info *j_b = ((job*)b)->metadata;
    if (j_a->run_time - j_b->run_time < 0) {
        return -1;
    } else if (j_a->run_time - j_b->run_time > 0){
        return 1;
    }
    return break_tie(a, b);
    return 0;
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    // TODO: Implement me!
    job_info *new_job = malloc(sizeof(job_info));
    new_job->id = job_number;
    new_job->arrival_time = time;
    new_job->run_time = sched_data->running_time;
    new_job->left_time = new_job->run_time;
    new_job->priority = sched_data->priority;
    new_job->last_run = -1;
    new_job->start_time = -1;
    new_job->end_time = -1;

    newjob->metadata = new_job;
    priqueue_offer(&pqueue, newjob);
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO: Implement me!
    if (!job_evicted) {
        return priqueue_peek(&pqueue);
    }

    job_info *j_info = job_evicted->metadata;
    j_info->last_run = time;
    j_info->left_time--;
    if (j_info->start_time < 0) {
        j_info->start_time = time - 1;
    }

    if (pqueue_scheme != PPRI || pqueue_scheme != PSRTF || pqueue_scheme != RR) {
        return job_evicted;
    } else {
        job *j = priqueue_poll(&pqueue);
        priqueue_offer(&pqueue, j);
        return priqueue_peek(&pqueue);
    }
}

static int jobs = 0;
static double total_wait_time = 0;
static double total_turnaround_time = 0;
static double total_response_time = 0;

void scheduler_job_finished(job *job_done, double time) {
    // TODO: Implement me!
    job_info *j = job_done->metadata;
    total_wait_time += time - j->arrival_time - j->run_time;
    total_turnaround_time += time - j->arrival_time;
    total_response_time += j->start_time - j->arrival_time;
    free(j);
    priqueue_poll(&pqueue);
    jobs++;
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO: Implement me!
    return total_wait_time / jobs;
}

double scheduler_average_turnaround_time() {
    // TODO: Implement me!
    return total_turnaround_time / jobs;
}

double scheduler_average_response_time() {
    // TODO: Implement me!
    return total_response_time / jobs;
}

void scheduler_show_queue() {
    // OPTIONAL: Implement this if you need it!
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}
