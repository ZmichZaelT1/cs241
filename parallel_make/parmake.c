/**
 * parallel_make
 * CS 241 - Spring 2022
 */

#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "vector.h"
#include "set.h"
#include "queue.h"
#include "set.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>


graph *g;
graph *g_ref;
set *is_visited;
queue *q;
set *s;
graph *rules_graph;
vector *goals;
pthread_cond_t c;
pthread_mutex_t m;
pthread_barrier_t b;
int curr_tasks_num = 0;
int graph_changed = 1;

int checkCyclic(char*);
int run_goal(char*);
int isNewer(rule_t*, rule_t*);
int run_command(rule_t*);
void *parl_run(void*);
vector *get_leaves(graph*, char*);
int check_if_dependent(graph*, char*, char*);
void push_leaves_to_queue(vector*);

//todo: 5, 7

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    g = parser_parse_makefile(makefile, targets);
    g_ref = parser_parse_makefile(makefile, targets);
    q = queue_create(0);
    s = shallow_set_create();
    rules_graph = shallow_graph_create();
    pthread_cond_init(&c, NULL);
    pthread_mutex_init(&m, NULL);
    pthread_t pid[num_threads];
    for (size_t i = 0; i < num_threads; i++) {
        pthread_create(pid+i, NULL, parl_run, NULL);
    }

    // pthread_barrier_init(&b, NULL, num_threads);

    goals = graph_neighbors(g, "");
    for (size_t i = 0; i < vector_size(goals); i++) {
        char *goal = vector_get(goals, i);
        if (checkCyclic(goal)) {
            print_cycle_failure(goal);
            rule_t *rule = (rule_t *) graph_get_vertex_value(g, goal);
            rule->state = -1;
        } else {
            // run_goal(goal);
            char *tmp = calloc(1, strlen(goal)+1);
            tmp = strdup(goal);
            while (1) {
                pthread_mutex_lock(&m);
                while (!graph_changed) {
                    pthread_cond_wait(&c, &m);
                }
                if (strcmp(tmp, goal)) break;
                if (!graph_contains_vertex(g, goal)){
                    // pthread_mutex_unlock(&m);
                    break;
                }
                vector *leaves = get_leaves(g, goal);
                // for (size_t j = 0; j < vector_size(leaves); j++) {
                //     char *tmp = vector_get(leaves,j);
                //     printf("vector has: %s\n", tmp);
                // }
                // puts("\n\n\n");
                curr_tasks_num = vector_size(leaves);
                if (vector_size(leaves) == 0) {
                    break;
                }
                push_leaves_to_queue(leaves);
                // puts("lock");
                graph_changed = 0;
                pthread_mutex_unlock(&m);
                // puts("not lock");
                vector_destroy(leaves);
                // while (curr_tasks_num != 0) {
                //     pthread_cond_wait(&c, &m);
                // }
            }
            free(tmp);
        }
    }
    queue_push(q, NULL);

    for (size_t i = 0; i < num_threads; i++) {
        pthread_join(pid[i], NULL);
    }

    // while (1) {
    //     rule_t *r = queue_pull(q);
    //     if (!r) {
    //         break;
    //     }
    //     printf("%s\n", r->target);
    // }

    graph_destroy(g);
    graph_destroy(g_ref);
    queue_destroy(q);
    vector_destroy(goals);
    pthread_cond_destroy(&c);
    pthread_mutex_destroy(&m);
    // pthread_barrier_destroy(&b);
    return 0;
}

void *parl_run(void *v) {
    while (1) {
        char *t = queue_pull(q);
        if (!t) {
            queue_push(q, NULL);
            return NULL;
        }
        vector *neighbors = graph_neighbors(g_ref, t);
        rule_t *t_r = (rule_t*) graph_get_vertex_value(g_ref, t);
        int failed = 0;
        if (vector_size(neighbors) != 0) {
            for (size_t i = 0; i < vector_size(neighbors); i++) {
                char *neighbor = vector_get(neighbors, i);
                rule_t *rule = (rule_t*) graph_get_vertex_value(g_ref, neighbor);
                if (rule->state == -1) {
                    t_r->state = -1;
                    // vector_destroy(neighbors)
                    failed = 1;
                    break;
                }
            }
            if (!failed) {
                for (size_t i = 0; i < vector_size(neighbors); i++) {
                    char *neighbor = vector_get(neighbors, i);
                    rule_t *rule = (rule_t*) graph_get_vertex_value(g_ref, neighbor);
                    if (access(t_r->target, F_OK) == 0) {
                        if (access(rule->target, F_OK) == 0) {
                            if (!isNewer(rule, t_r)) {
                                t_r->state = 1;
                                break;
                            }
                        }
                    }
                }
                if (t_r->state != 1) {
                    int run_stat = run_command(t_r);
                    if (run_stat == -1) {
                        t_r->state = -1;
                    } else {
                        t_r->state = 1;
                    }
                }

            }
        } else {
            if (access(t_r->target, F_OK) == -1) {
                int run_stat = run_command(t_r);
                if (run_stat == -1) {
                    t_r->state = -1;
                } else {
                    t_r->state = 1;
                }
            }
        }
        vector_destroy(neighbors);



        // if (r->state != -1 && r->state != 1) {
        //     int run_stat = run_command(r); 
        //     if (run_stat == -1) {
        //         r->state = -1;
        //     } else {
        //         r->state = 1;
        //     }
        // }

        graph_remove_vertex(g, t);
        graph_changed = 1;
        pthread_cond_signal(&c);
    }
    return NULL;
}

void push_leaves_to_queue(vector *leaves) {
    for (size_t i = 0; i < vector_size(leaves); i++) {
        char *curr = vector_get(leaves, i);
        // rule_t *rule = (rule_t*) graph_get_vertex_value(g, curr);
        // if (access(rule->target, F_OK) == -1) {
            if (!set_contains(s, curr)) {
                // printf("pushed to queue: %s\n", curr);
                queue_push(q, curr);
                set_add(s, curr);
            }
        // }
    }
}

vector *get_leaves(graph *g, char *goal) {
    vector *ret = shallow_vector_create();
    vector *nodes = graph_vertices(g);
    for (size_t i = 0; i < vector_size(nodes); i++) {
        char *curr = vector_get(nodes, i);
        // if (!strcmp(curr, goal)) continue;
        if (!strcmp(curr, "")) {
            continue;
        }
        vector *neighbors = graph_neighbors(g, curr);
        if (vector_size(neighbors) == 0) { // curr is leaf
            if (check_if_dependent(g, curr, goal)) { // check if its goal's dependency
                // printf("pushed to vector: %s\n", curr);
                vector_push_back(ret, curr);
            }
        }
        vector_destroy(neighbors);
    }
    vector_destroy(nodes);
    return ret;
}

int check_if_dependent(graph *g, char *node, char *goal) {
    if (!strcmp(node, goal)) {
        return 1;
    }
    if (!graph_contains_vertex(g, goal)) {
        return 0;
    }
    vector *neighbors = graph_neighbors(g, goal);
    if (vector_size(neighbors) == 0) {
        vector_destroy(neighbors);
        return 0;
    }
    for (size_t i = 0; i < vector_size(neighbors); i++) {
        char *curr = vector_get(neighbors, i);
        if (strcmp(curr, node)) { // if not equal, continue searching
            if (check_if_dependent(g, node, curr)) {
                vector_destroy(neighbors);
                return 1;
            }
        } else {
            vector_destroy(neighbors);
            return 1;
        }
    }
    vector_destroy(neighbors);
    return 0;
}

int run_command(rule_t *rule) {
    for (size_t i = 0; i < vector_size(rule->commands); i++) {
        if (system(vector_get(rule->commands, i)) != 0) {
            return -1;
        }
    }
    return 1;
}

int checkCyclic(char *goal) {
    if(!is_visited) is_visited = shallow_set_create();
    if(set_contains(is_visited, goal)){
        set_destroy(is_visited);
        is_visited = NULL;
        return 1;
    } else {
        set_add(is_visited, goal);
        vector* reachables = graph_neighbors(g, goal);
        for(size_t i = 0; i < vector_size(reachables); i++){
            if(checkCyclic(vector_get(reachables, i))) {
                vector_destroy(reachables);
                return 1;
            }
        }
        vector_destroy(reachables);
    }
    set_destroy(is_visited);
    is_visited = NULL;
    return 0;
}

int isNewer(rule_t *neighbor, rule_t *goal) {
    struct stat neighbor_s;
    struct stat goal_s;
    stat(neighbor->target, &neighbor_s);
    stat(goal->target, &goal_s);
    if (difftime(neighbor_s.st_mtime, goal_s.st_mtime) > 0) {
        return 1;
    }
    return 0;
}


int run_goal(char *goal) {
    vector *neighbors = graph_neighbors(g, goal);
    rule_t *goal_rule = (rule_t*) graph_get_vertex_value(g, goal);
    int failed = 0;
    if (vector_size(neighbors) != 0) {
        for (size_t i = 0; i < vector_size(neighbors); i++) {
            char *neighbor = vector_get(neighbors, i);
            rule_t *rule = (rule_t*) graph_get_vertex_value(g, neighbor);
            if (rule->state == -1) {
                goal_rule->state = -1;
                vector_destroy(neighbors);
                return -1;
            } else if (rule->state == 0){
                int s = run_goal(neighbor);
                if (s == -1) {
                    goal_rule->state = -1;
                    // return -1;
                    failed = 1;
                }
            }
        }
        if (failed) {
            vector_destroy(neighbors);
            return -1;
        }
        for (size_t i = 0; i < vector_size(neighbors); i++) {
            char *neighbor = vector_get(neighbors, i);
            rule_t *rule = (rule_t*) graph_get_vertex_value(g, neighbor);
            if (access(goal_rule->target, F_OK) == 0) {
                if (access(rule->target, F_OK) == 0) {
                    if (!isNewer(rule, goal_rule)) {
                        goal_rule->state = 1;
                        vector_destroy(neighbors);
                        return 1;
                    }
                }
            }
            // int run_stat = run_command(goal_rule);
            if (!set_contains(s, goal_rule)) {
                queue_push(q, goal_rule);
                set_add(s, goal_rule);
            }
            // queue_push(q, goal_rule);
            // vector_destroy(neighbors);
            // if (run_stat == -1) {
            //     goal_rule->state = -1;
            //     vector_destroy(neighbors);
            //     return -1;
            // } else {
            //     goal_rule->state = 1;
            //     vector_destroy(neighbors);
            //     return 1;
            // }
        }
    } else {
        if (access(goal_rule->target, F_OK) == -1) {
            // int run_stat = run_command(goal_rule);
            if (!set_contains(s, goal_rule)) {
                queue_push(q, goal_rule);
                set_add(s, goal_rule);
            }
                        // vector_destroy(neighbors);
            // if (run_stat == -1) {
            //     goal_rule->state = -1;
            //     vector_destroy(neighbors);
            //     return -1;
            // } else {
            //     goal_rule->state = 1;
            //     vector_destroy(neighbors);
            //     return 1;
            // }
        }
    }
    vector_destroy(neighbors);
    return 1;
}
