/**
 * mini_memcheck
 * CS 241 - Spring 2022
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>

meta_data *head = NULL;
size_t total_memory_requested = 0;
size_t total_memory_freed = 0;
size_t invalid_addresses = 0;

int check_addr(void*);

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here
    if (!request_size) return NULL;
    meta_data *mem = malloc(request_size + sizeof(meta_data));
    if (!mem) return NULL;
    mem->request_size = request_size;
    mem->filename = filename;
    mem->instruction = instruction;
    mem->next = NULL;

    if (head) {
        meta_data *tmp = head;
        while (tmp->next != NULL) tmp = tmp->next;
        tmp->next = mem;
    } else {
        head = mem;
    }
    total_memory_requested += request_size;
    return (void*) (mem + 1);
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    size_t total_size = num_elements * element_size;
    if (!total_size) return NULL;
    void *mem = mini_malloc(total_size, filename, instruction);
    if (!mem) return NULL;
    return memset(mem, 0, total_size); 
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    if (!check_addr(payload)) {invalid_addresses++; return NULL;}
    if (!payload && !request_size) return NULL;
    if (!payload) return mini_malloc(request_size, filename, instruction);
    if (!request_size) {mini_free(payload); return NULL;}

    meta_data *mem = (meta_data*) payload - 1;
    meta_data *realloc_mem = realloc(mem, request_size + sizeof(meta_data));
    if (!realloc_mem) return NULL;
    
    if (mem == head) {
        head = realloc_mem;
    } else {
        meta_data *tmp = head;
        while (tmp->next != mem) {
            tmp = tmp->next;
        }
        tmp->next = realloc_mem;
    }

    if (request_size <= realloc_mem->request_size) {
        total_memory_freed += realloc_mem->request_size - request_size;
    } else {
        total_memory_requested += request_size - realloc_mem->request_size;
    }

    realloc_mem->request_size = request_size;
    
    return (void*) (realloc_mem + 1);
}

void mini_free(void *payload) {
    // your code here
    if (!payload) return;
    if (!check_addr(payload)) {invalid_addresses++; return;}
    meta_data *mem = (meta_data*) payload - 1;
    if (mem == head) {
        head = mem->next;
    } else {
        meta_data *tmp = head;
        while (tmp->next != mem) {
            tmp = tmp->next;
        }
        tmp->next = mem->next;
    }
    total_memory_freed += mem->request_size;
    free(mem);
}

int check_addr(void *payload) {
    if (!head) return 0;
    meta_data *tmp = head;
    while (tmp) {
        if (tmp + 1 == payload) return 1;
        tmp = tmp->next;
    }
    return 0;
}