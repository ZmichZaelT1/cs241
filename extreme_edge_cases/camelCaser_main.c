/**
 * extreme_edge_cases
 * CS 241 - Spring 2022
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int main() {
    // Feel free to add more test cases of your own!
    // char *empty = "";
    // char *test = malloc(sizeof(char)*5);
    // strcpy(test,empty);
    // free(test);

    // char *input = "hello\n welco\nme .1to cs241";
    // char **output = camel_caser(input);
    // while (*output) {
    //     printf("output: %s\n", *output);
    //     output++;
    // }

    if (test_camelCaser(&camel_caser, &destroy)) {
        printf("SUCCESS\n");
    } else {
        printf("FAILED\n");
    }
}
