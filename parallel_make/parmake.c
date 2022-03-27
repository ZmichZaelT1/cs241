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
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>

graph *g;
set *is_visited;
queue *q;

int checkCyclic(char*);
int run_goal(char*);
int isNewer(rule_t*, rule_t*);
int run_command(rule_t*);

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    g = parser_parse_makefile(makefile, targets);
    q = queue_create(0);

    vector *goals = graph_neighbors(g, "");
    for (size_t i = 0; i < vector_size(goals); i++) {
        char *goal = vector_get(goals, i);
        if (checkCyclic(goal)) {
            print_cycle_failure(goal);
            rule_t *rule = (rule_t *) graph_get_vertex_value(g, goal);
            rule->state = -1;
        }
    }

    for (size_t i = 0; i < vector_size(goals); i++) {
        char *goal = vector_get(goals, i);
        rule_t *rule = (rule_t *) graph_get_vertex_value(g, goal);
        if (rule->state != -1) {
            run_goal(goal);
        }
    }
    graph_destroy(g);
    queue_destroy(q);
    vector_destroy(goals);
    return 0;
}

// void destroy_stuff()

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
            int run_stat = run_command(goal_rule);
            if (run_stat == -1) {
                goal_rule->state = -1;
                vector_destroy(neighbors);
                return -1;
            } else {
                goal_rule->state = 1;
                vector_destroy(neighbors);
                return 1;
            }
        }
    } else {
        if (access(goal_rule->target, F_OK) == -1) {
            int run_stat = run_command(goal_rule);
            if (run_stat == -1) {
                goal_rule->state = -1;
                vector_destroy(neighbors);
                return -1;
            } else {
                goal_rule->state = 1;
                vector_destroy(neighbors);
                return 1;
            }
        }
    }
    vector_destroy(neighbors);
    return 1;
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
