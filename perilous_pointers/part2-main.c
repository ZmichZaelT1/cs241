/**
 * perilous_pointers
 * CS 241 - Spring 2022
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);

    int v2 = 132;
    second_step(&v2);

    int v3 = 8942;
    int *dv3 = &v3;
    double_step(&dv3);

    char v4[] = {0,1,2,0,4,15, 0, 0, 0};
    strange_step(v4);

    empty_step(v4);

    char v6[] = {'u', 'u', 'u', 'u'};
    two_step(v6, v6);
    // void v5[] = {0,0,0,0}; 

    char first[] = {0,1,2,3,4};
    char* second = first + 2;
    char* third = second + 2;
    three_step(first, second, third);

    char first8[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
    char* second8 = first8+7;
    char* third8 = second8 + 7;
    step_step_step(first8, second8, third8);

    char a[] = {5};
    int b = 5;
    it_may_be_odd(a, b);

    char s10[20] = "a,CS241";
    tok_step(s10);

    char blue[] = {1,0,0,5,0,0,0,0};
    char* orange = blue;
    the_end(orange, blue);

    return 0;



    // char *s10 = "a, CS241, b";
    // char s10[20] = "a,CS241";
    // char *aaa = strtok(s10, ",");
    // aaa = strtok(NULL, ",");
    // printf("a: %s", aaa);
}
