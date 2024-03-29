/**
 * extreme_edge_cases
 * CS 241 - Spring 2022
 */
#include <stdio.h>

#include "camelCaser_ref_utils.h"
#include "camelCaser.h"

int main() {

    char *test1 = "216asd@$#mkhN# kNlk#$L^mLK3$nklL#$mkl5l#$KmLnm 5# L#K#$jkl #$ljk#$4lk#$ L#$lk $3 LK$# L#$lkj$ #Lkj l#$ kj3$3kj4# #$KL34 L#$lk34j#$# KL$ 3 $3kL $$# L#$ #$ LK#$JK#Jk34kl3l34#$ KLj";
    puts("test1: \n");
    print_camelCaser(test1);



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
