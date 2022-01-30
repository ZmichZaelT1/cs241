/**
 * extreme_edge_cases
 * CS 241 - Spring 2022
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>


void removeSpaces(char*);
int findBufferSize(char*);
void adjustCase(char* str);
char **camel_caser(const char *input_str) {
    // TODO: Implement me!
    if (input_str == NULL) {
        return NULL;
    }
    char* i = malloc(strlen(input_str) + 1);
    char* toFree = i;
    strcpy(i,input_str);
    printf("copy : %s\n", i);
    adjustCase(i);
    // printf("upper and lower cases: %s\n", i);

    removeSpaces(i);

    int bufferSize = findBufferSize(i);
    // printf("buffer size is %d\n", bufferSize);
    char** output = malloc(sizeof(char*) * bufferSize);
    for (int a = 0; a < bufferSize; a++) {
        output[a] = NULL;
    }

    int count = 0;
    int isValid = !ispunct(i[0]);
    int started = 0;
    int len = 0;
    int emptyStr = 0;
    int isFirstPunc = 1;
    char *tmp;
    if (!strcmp(i, "")) {
        *output = NULL;
        free(toFree);
        return output;
    }
    while (*i) {
        if (!ispunct(*(i))) {
            if (isValid) {
                tmp = i;
                started = 1;
                isValid = 0;
                len++;
                emptyStr = 0;
            } else {
                len++;
                emptyStr = 0;
            }
        }
        else if (ispunct(*i)) {
            *i = '\0';
            if (!isValid &&started) {
                output[count] = malloc(sizeof(char) * (len+1));
                strcpy(output[count], tmp);
                started = 0;
                count++;
            }
            else if (emptyStr) {
                output[count] = malloc(sizeof(char));
                strcpy(output[count], i-1);
                count++;
            }
            else if (isFirstPunc) {
                output[count] = malloc(sizeof(char));
                strcpy(output[count], i);
                isFirstPunc = 0;
                count++;
            }
            isValid = 1;
            len = 0;
            emptyStr = 1;
        } 
        i++;
    }
    free(toFree);
    return output;
}

void destroy(char **result) {
    // TODO: Implement me!
    if (!result) return;
    char** tmp = result;
    while (*tmp) {
        free(*tmp);
        tmp++;
    }
    free(result);
    return;
}

void removeSpaces(char *str) {
    int count = 0;
    for (int i = 0; str[i]; i++) {
        if (!isspace(str[i])) {
            str[count] = str[i];
            count++;
        }
    }
    str[count] = '\0';
}

int findBufferSize(char *str) {
    int count = 0;
    int isValid = !ispunct(*str);
    int started = 0;
    int emptyStr = 0;
    int isFirstPunc = 1;
    int i;
    for (i = 0; str[i]; i++) {
        if(!ispunct(str[i])) {
            if (isValid) {
                started = 1;
                isValid = 0;
                emptyStr = 0;
            }
        }
        else if (ispunct(str[i])) {
            if (started && !isValid) {
                started = 0;
                count++;
            }
            else if(emptyStr) {
                count++;
                isValid = 1;
            }
            else if (isFirstPunc) {
                isFirstPunc = 0;
                count++;
            }
            isValid = 1;
            emptyStr = 1;
        }
    }
    return count+1;
}

void adjustCase(char* str) {
    int j;
    for (j = 0; str[j]; j++) {
        str[j] = tolower(str[j]);
    }

    int firstWordFounded = 0;
    int newSentence = 1;
    int isFirstChar = 1;
    int i;
    for (i = 0; str[i]; i++) {
        if (ispunct(str[i])) {
            newSentence = 1;
            firstWordFounded = 0;
            isFirstChar = 1;
        }
        else if (isalpha(str[i])) {
            if (firstWordFounded) {
                if (isFirstChar) {
                    str[i] = toupper(str[i]);
                    isFirstChar = 0;
                }
            }
            firstWordFounded = 1;
            isFirstChar = 0;
        }
        else if (isspace(str[i])) {
            isFirstChar = 1;
        } 
        else if (isdigit(str[i])) {
            if (!firstWordFounded) {
                firstWordFounded = 1;
                isFirstChar = 0;
            }
        }
    }
}
