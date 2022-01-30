/**
 * extreme_edge_cases
 * CS 241 - Spring 2022
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"
int check_output(char **, char **);
int size(char **);
int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {

// simple case
    char *test1 = "hello. welcome to cs241.";
    char **result1 = camelCaser(test1);
    char *expected1[] = {"hello", "welcomeToCs241", NULL};
    if (!check_output(expected1, result1)) {
        printf("failed test 1");
        return 0;
    }
    printf("check_output test 1: %d\n", check_output(expected1, result1));
    destroy(result1);
//multiple spaces
    char *test2 = "  hel  lo.   we lco me t o c s24   1  ";
    char **result2 = camelCaser(test2);
    char *expected2[] = {"helLo", NULL};
    if (!check_output(expected2, result2)) {
        printf("failed test 2");
        return 0;
    }
    printf("check_output test 2: %d\n", check_output(expected2, result2));
    destroy(result2);

// multiple punct
    char *test3 = "hello#@. welcome#$ to cs241.";
    char **result3 = camelCaser(test3);
    char *expected3[] = {"hello", "", "", "welcome", "", "toCs241", NULL};
    if (!check_output(expected3, result3)) {
        printf("failed test 3");
        return 0;
    }
    printf("check_output test 3: %d\n", check_output(expected3, result3));
    destroy(result3);

// empty
    char *test4 = "";
    char **result4 = camelCaser(test4);
    char *expected4[] = {NULL};
    if (!check_output(expected4, result4)) {
        printf("failed test 4");
        return 0;
    }
    printf("check_output test 4: %d\n", check_output(expected4, result4));
    destroy(result4);

// multi empty
    char *test5 = "      ";
    char **result5 = camelCaser(test5);
    char *expected5[] = {NULL};
    if (!check_output(expected5, result5)) {
        printf("failed test 5");
        return 0;
    }
    printf("check_output test 5: %d\n", check_output(expected5, result5));
    destroy(result5);

// start with punc
    char *test6 = "@hello. welcome .to cs241";
    char **result6 = camelCaser(test6);
    char *expected6[] = {"", "hello", "welcome",NULL};
    if (!check_output(expected6, result6)) {
        printf("failed test 6");
        return 0;
    }
    printf("check_output test 6: %d\n", check_output(expected6, result6));
    destroy(result6);

// Test upper cases
    char *test7 = "HELLO WELCOME .to cs241";
    char **result7 = camelCaser(test7);
    char *expected7[] = {"helloWelcome",NULL};
    if (!check_output(expected7, result7)) {
        printf("failed test 7");
        return 0;
    }
    printf("check_output test 7: %d\n", check_output(expected7, result7));
    destroy(result7);

// Test lower cases
    char *test8 = "hello welcome .to cs241";
    char **result8 = camelCaser(test8);
    char *expected8[] = {"helloWelcome",NULL};
    if (!check_output(expected8, result8)) {
        printf("failed test 8");
        return 0;
    }
    printf("check_output test 8: %d\n", check_output(expected8, result8));
    destroy(result8);

// test start with numbers
    char *test9 = "12hello welcome .1to cs241";
    char **result9 = camelCaser(test9);
    char *expected9[] = {"12helloWelcome",NULL};
    if (!check_output(expected9, result9)) {
        printf("failed test 9");
        return 0;
    }
    printf("check_output test 9: %d\n", check_output(expected9, result9));
    destroy(result9);

// test start with numbers
    char *test10 = "hello\n welco\nme .1to cs241";
    char **result10 = camelCaser(test10);
    char *expected10[] = {"helloWelcoMe", NULL};
    if (!check_output(expected10, result10)) {
        printf("failed test 10");
        return 0;
    }
    printf("check_output test 10: %d\n", check_output(expected10, result10));
    destroy(result10);

// test NULL
    char *test11 = NULL;
    char **result11 = camelCaser(test11);
    char *expected11[] = {NULL};
    if (!check_output(expected11, result11)) {
        printf("failed test 11");
        return 0;
    }
    printf("check_output test 11: %d\n", check_output(expected11, result11));
    destroy(result11);

// test numbers
    char *test12 = "013Ddaf sd1Sa 4sd.s fsd56f5 sd534 s3d1.sd5f4 fde6wehb31gh .j3hg50 1j5 .";
    char **result12 = camelCaser(test12);
    char *expected12[] = {"013ddafSd1sa4Sd","sFsd56f5Sd534S3d1", "sd5f4Fde6wehb31gh", "j3hg501J5", NULL};
    if (!check_output(expected12, result12)) {
        printf("failed test 12");
        return 0;
    }
    printf("check_output test 12: %d\n", check_output(expected12, result12));
    destroy(result12);

// test random super long str
    char *test13 = "216asd@$#mkhN# kNlk#$L^mLK3$nklL#$mkl5l#$KmLnm 5# L#K#$jkl #$ljk#$4lk#$ L#$lk $3 LK$# L#$lkj$ #Lkj l#$ kj3$3kj4# #$KL34 L#$lk34j#$# KL$ 3 $3kL $$# L#$ #$ LK#$JK#Jk34kl3l34#$ KLj";
    char **result13 = camelCaser(test13);
    char *expected13[] = {"216asd","","","mkhn","knlk","","l","mlk3","nkll","","mkl5l","","kmlnm5","l","k","","jkl","","ljk","","4lk","","l","","lk","3Lk","","l","","lkj","","lkjL","","kj3","3kj4","","","kl34L","","lk34j","","","kl","3","3kl","","","l","","","","lk","","jk","jk34kl3l34","",NULL};
    if (!check_output(expected13, result13)) {
        printf("failed test 13");
        return 0;
    }
    printf("check_output test 13: %d\n", check_output(expected13, result13));
    destroy(result13);

    return 1;

}


int check_output(char **answer, char **result) {
    if (size(answer) != size(result)) {
        return 0;
    }

    int i;
    for (i = 0; answer[i]; i++) {
        if (strcmp(answer[i], result[i])) {
            return 0;
        }
    }
    return 1;
}

int size(char **input) {
    if (input == NULL) {
        return 0;
    }
    int size = 0;
    char **tmp = input;
    while (*tmp) {
        ++size;
        ++tmp;
    }
    return size;
}

