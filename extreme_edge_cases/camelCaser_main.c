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

    // char *input = "Try 2 NOT end with a punctuat1ON, start counting from 2 to 10: one, two, three, four";
    // char **output = camel_caser(input);
    // char** tmp = output;
    // while (*output) {
    //     printf("output: %s\n", *output);
    //     output++;
    // }
    // destroy(tmp);

    if (test_camelCaser(&camel_caser, &destroy)) {
        printf("SUCCESS\n");
    } else {
        printf("FAILED\n");
    }
}
