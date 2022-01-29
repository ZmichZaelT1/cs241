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

    char* i = strdup(input_str);
    printf("copy : %s\n", i);
    adjustCase(i);
    // printf("upper and lower cases: %s\n", i);

    removeSpaces(i);

    int bufferSize = findBufferSize(i);
    // printf("buffer size is %d\n", bufferSize);
    char** output = malloc(sizeof(char*) * bufferSize);

    int count = 0;
    int isValid = !ispunct(i[0]);
    int started = 0;
    int len = 0;
    int emptyStr = 0;
    char *tmp;
    while (*i) {
        if (isValid && !ispunct(*i)) {
            tmp = i;
            started = 1;
            isValid = 0;
            len++;
            emptyStr = 0;
        }
        else if (!isValid &&started && ispunct(*i)) {
            output[count] = malloc(sizeof(char) * (len+1));
            output[count] = tmp;
            started = 0;
            count++;
            *i = '\0';
            isValid = 1;
            len = 0;
            emptyStr = 1;
        }

        else if (emptyStr && ispunct(*i)) {
            output[count] = malloc(sizeof(char));
            output[count] = i-1;
            count++;
            *i = '\0';
            isValid = 1;
            len = 0;
        }
        else if (ispunct(*i)) {
            *i = '\0';
            isValid = 1;
            len = 0;
            emptyStr = 1;
        }
        else {
            len++;
            emptyStr = 0;
        }

        i++;
    }
    return output;
}

void destroy(char **result) {
    // TODO: Implement me!
    if (!result) return;
    char** tmp = result;
    while (*tmp) {
        if (!strcmp(*tmp, "")) {
            tmp++;
            continue;
        };
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

    int i;
    for (i = 0; str[i]; i++) {
        if(isValid && !ispunct(str[i])) {
            started = 1;
            isValid = 0;
            emptyStr = 0;
        }
        else if (started && !isValid && ispunct(str[i])) {
            started = 0;
            count++;
            isValid = 1;
            emptyStr = 1;
        }
        else if (emptyStr && ispunct(str[i])) {
            count++;
            isValid = 1;
        }
        else if (ispunct(str[i])) {
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
    }
}
