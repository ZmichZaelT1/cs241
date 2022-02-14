/**
 * mini_memcheck
 * CS 241 - Spring 2022
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Your tests here using malloc and free
    // void *p1 = malloc(30);
    // void *p2 = malloc(40);
    // void *p3 = malloc(50);
    // free(p2);

// bad_free_test
    // void *p1 = malloc(30);
    // void *p2 = realloc(p1+235, 35);
    // free(p1+33);

// calloc_test
    // void *p1 = calloc(30,20);
    // free(p1);

// leak_test
    // void *p1 = calloc(30,20);
    // void *p2 = malloc(40);
    // void *p3 = malloc(50);
    // free(p2);

// realloc_test
    void *p1 = calloc(30,20);
    void *p2 = malloc(40);
    void *p3 = realloc(p1, 50);
    void *p4 = realloc(p2, 50);
    free(p3);
    free(p4);

// double free
    // void *p1 = calloc(30,20);
    // free(p1);
    // free(p1);
    // return 0;
}