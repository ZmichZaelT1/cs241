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

    if (test_camelCaser(&camel_caser, &destroy)) {
        printf("SUCCESS\n");
    } else {
        printf("FAILED\n");
    }

// below for test:
    // int i = 0;
    // if (i) {
    //     char *input = "";
    //     char **output = camel_caser(input);
    //     char** tmp = output;
    //     while (*output) {
    //         printf("output: %s\n", *output);
    //         output++;
    //     }
    //     destroy(tmp);
    // } else {
    //     if (test_camelCaser(&camel_caser, &destroy)) {
    //         printf("SUCCESS\n");
    //     } else {
    //         printf("FAILED\n");
    //     }
    // }
}
