/**
 * vector
 * CS 241 - Spring 2022
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    char *str;
    size_t size;
};

sstring *cstr_to_sstring(const char *input) {
    // your code goes here
    sstring* str = malloc(sizeof(sstring));
    str->str = strdup(input);
    str->size = strlen(input);
    return str;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    char *ret = strdup(input->str);
    return ret;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    this->str = realloc(this->str, this->size + addition->size + 1);
    strcpy(this->str + this->size, addition->str);
    this->size += addition->size;
    return this->size;
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    vector* vec = string_vector_create();
    char* t = strdup(this->str);
    char* tmp = t;
    char* startOfWord = tmp;
    while (*tmp) {
        if (*tmp == delimiter) {
            *tmp = '\0';
            char* tmppp = strdup(startOfWord);
            vector_push_back(vec, tmppp);
            free(tmppp);
            if (*(tmp+1)) {
                startOfWord = tmp + 1;
            } else {
                vector_push_back(vec, "");
                free(t);
                return vec;
            }
        }
        tmp++;
    }
    char* tmpppp = strdup(startOfWord);
    vector_push_back(vec, tmpppp);
    free(tmpppp);
    free(t);
    return vec;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    if (offset > this->size) return -1;
    char * startTargetARest = strstr(this->str+offset, target);
    if (!startTargetARest) return -1;
    char * newStr = malloc(this->size - strlen(target) + strlen(substitution) + 1);
    strncpy(newStr, this->str, this->size - strlen(startTargetARest));
    newStr[this->size - strlen(startTargetARest)] = '\0';
    char * endTargetARest = startTargetARest+ strlen(target);
    strcat(newStr, substitution);
    strcat(newStr, endTargetARest);
    free(this->str);
    this->str = newStr;
    this->size = this->size - strlen(target) + strlen(substitution);

    return 0;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    char *ret = malloc(end - start + 1);
    strncpy(ret, this->str + start, end-start);
    ret[end-start] = '\0';
    return ret;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    free(this->str);
    free(this);
}
