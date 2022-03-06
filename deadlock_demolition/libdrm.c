/**
 * deadlock_demolition
 * CS 241 - Spring 2022
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

struct drm_t { pthread_mutex_t m; };

static graph *g = NULL;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
set *is_visited;

int checkCyclic(graph *g, set *is_visited, pthread_t *thread_id) {
    if(!is_visited) is_visited = shallow_set_create();
    if(set_contains(is_visited, thread_id)){
        is_visited = NULL;
        return 1;
    } else {
        set_add(is_visited, thread_id);
        vector* reachables = graph_neighbors(g, thread_id);
        for(size_t i = 0; i < vector_size(reachables); i++){
            if(checkCyclic(g, is_visited, vector_get(reachables, i))) return 1;
        }
    }

    is_visited = NULL;
    return 0;
}

drm_t *drm_init() {
    /* Your code here */
    drm_t *ret = calloc(1, sizeof(drm_t));
    pthread_mutex_init(&ret->m, NULL);
    pthread_mutex_lock(&m);
    if (!g) g = shallow_graph_create();
    graph_add_vertex(g, ret);
    pthread_mutex_unlock(&m);

    return ret;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    pthread_mutex_lock(&m);
    if(graph_contains_vertex(g, thread_id) && graph_contains_vertex(g, drm)) {
        if(graph_adjacent(g, drm, thread_id)){
            graph_remove_edge(g, drm, thread_id);
            pthread_mutex_unlock(&drm->m);
            pthread_mutex_unlock(&m);
            return 1;
        }
    }
    pthread_mutex_unlock(&m);
    return 0;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    pthread_mutex_lock(&m);
    graph_add_vertex(g, thread_id);
    if(!graph_adjacent(g, drm, thread_id)) {
        graph_add_edge(g, thread_id, drm);
        if (!checkCyclic(g, is_visited ,thread_id)) {
            pthread_mutex_unlock(&m);
            pthread_mutex_lock(&drm->m);
            pthread_mutex_lock(&m);
            graph_remove_edge(g, thread_id, drm);
            graph_add_edge(g, drm, thread_id);
            pthread_mutex_unlock(&m);    
            return 1;
        } else {
            graph_remove_edge(g, thread_id, drm);
            pthread_mutex_unlock(&m);
            return 0;
        }
    } else {
        pthread_mutex_unlock(&m);
        return 0;
    }
}

void drm_destroy(drm_t *drm) {
    /* Your code here */
    graph_remove_vertex(g, drm);
    pthread_mutex_destroy(&m);
    pthread_mutex_destroy(&drm->m);
    free(drm);
    return;
}
