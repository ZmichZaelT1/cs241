/**
 * vector
 * CS 241 - Spring 2022
 */
#include "vector.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[]) {
    // Write your test cases here

// test create
    vector* v1 = int_vector_create();
    assert(vector_begin(v1));
    assert(vector_end(v1));
    assert(!vector_size(v1));
    assert(vector_capacity(v1) == 8);
    assert(vector_empty(v1));

// test push back
    int a[] = {0,1,2,3,4,5,6,7,8};
    for (int i = 0; i < 9; i++) vector_push_back(v1, a+i);
    assert(vector_size(v1) == 9);
    assert(vector_capacity(v1) == 16);
    assert(!vector_empty(v1));
    assert(*(int*)vector_get(v1, 3) == 3);
    assert(*(int*)vector_get(v1, 8) == 8);

// Test pop back
    for (int i = 3; i > 0; i--) vector_pop_back(v1);
    assert(vector_size(v1) == 6);
    assert(vector_capacity(v1) == 16);
    assert(!vector_empty(v1));
    assert(*(int*)vector_get(v1, 3) == 3);
    assert(*(int*)vector_get(v1, 5) == 5);
    for (int i = 6; i > 0; i--) vector_pop_back(v1);
    assert(vector_empty(v1));

// test resize less
    vector* v2 = int_vector_create();
    int b[] = {0,1,2,3,4,5,6,7,8};
    for (int i = 0; i < 9; i++) vector_push_back(v2, b+i);
    vector_resize(v2, 4);
    assert(vector_size(v2) == 4);
    assert(vector_capacity(v2) == 16);
    assert(!vector_empty(v2));
    assert(*(int*)vector_get(v2, 2) == 2); 

// test resize greater
    vector_resize(v2, 16);
    assert(vector_size(v2) == 16);
    assert(vector_capacity(v2) == 16);
    assert(!vector_empty(v2));
    assert(*(int*)vector_get(v2, 2) == 2); 

    vector_resize(v2, 17);
    assert(vector_size(v2) == 17);
    assert(vector_capacity(v2) == 32);
    assert(!vector_empty(v2));
    assert(*(int*)vector_get(v2, 2) == 2);

// test resize 0
    vector_resize(v2,0);
    assert(vector_size(v2) == 0);
    assert(vector_capacity(v2) == 32);
    assert(vector_empty(v2));

// test erase
    vector* v3 = int_vector_create();
    int c[] = {0,1,2,3};
    for (int i = 0; i < 4; i++) vector_push_back(v3, c+i);
    vector_erase(v3, 3);
    assert(vector_size(v3) == 3);
    assert(vector_capacity(v3) == 8);
    assert(*(int*)vector_get(v3, 2) == 2);
    vector_erase(v3, 2);
    vector_erase(v3, 1);
    vector_erase(v3, 0);
    assert(vector_size(v3) == 0);
    assert(vector_capacity(v3) == 8);
    assert(vector_empty(v3));

// test clear
    vector* v4 = int_vector_create();
    int d[] = {0,1,2,3};
    for (int i = 0; i < 4; i++) vector_push_back(v4, d+i);
    vector_clear(v4);
    assert(vector_empty(v4));
    assert(vector_capacity(v4) == 8);

    vector_destroy(v1);
    vector_destroy(v2);
    vector_destroy(v3);
    vector_destroy(v4);


    return 0;
}
