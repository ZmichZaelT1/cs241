/**
 * extreme_edge_cases
 * CS 241 - Spring 2022
 */
#include <stdio.h>

#include "camelCaser_ref_utils.h"
#include "camelCaser.h"

int main() {

    char *test1 = "hello\n welco\nme .1to cs241";
    char *test2 = " hel  lo.   we lco me t o c s24   1  ";
    char *test3 = "hello#@. welcome#$ to cs241.";
    char *test4 = "";
    char *test5 = "      ";
    char *test6 = "@#hello. welcome .to cs241";
    char *test7 = NULL;

    puts("test1: \n");
    print_camelCaser(test1);
    puts("test2: \n");
    print_camelCaser(test2);
    puts("test3: \n");
    print_camelCaser(test3);
    puts("test4: \n");
    print_camelCaser(test4);
    puts("test5: \n");
    print_camelCaser(test5);
    puts("test6: \n");
    print_camelCaser(test6);
    puts("test7: \n");
    print_camelCaser(test7);



    // Enter the string you want to test with the reference here.
    char *input = "hello. welcome to cs241";

    // This function prints the reference implementation output on the terminal.
    print_camelCaser(input);

    // Put your expected output for the given input above.
    char *correct[] = {"hello", NULL};
    char *wrong[] = {"hello", "welcomeToCs241", NULL};

    // Compares the expected output you supplied with the reference output.
    printf("check_output test 1: %d\n", check_output(input, correct));
    printf("check_output test 2: %d\n", check_output(input, wrong));

    // Feel free to add more test cases.
    return 0;
}
