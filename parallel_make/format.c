/**
 * parallel_make
 * CS 241 - Spring 2022
 */
#include "format.h"
#include <stdio.h>

void print_cycle_failure(char *target) {
    printf("parmake: dropped goal '%s' due to circular dependencies\n", target);
    fflush(stdout);
}
