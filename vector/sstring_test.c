/**
 * vector
 * CS 241 - Spring 2022
 */
#include "sstring.h"
#include <assert.h>
#include <string.h>
int main(int argc, char *argv[]) {
    // TODO create some tests
    sstring *str1 = cstr_to_sstring("abc");
    char *str1_c = sstring_to_cstr(str1);
    assert(!strcmp(str1_c, "abc"));
    free(str1_c);
    sstring *str2 = cstr_to_sstring("def");
    int size = sstring_append(str1, str2);
    assert(size == 6);
    char *tmp = sstring_to_cstr(str1);
    assert(!strcmp(tmp, "abcdef"));
    free(tmp);

    vector *v1 = sstring_split(str1, 'c');
    assert(vector_size(v1) == 2);
    assert(!strcmp((char*)vector_get(v1, 0), "ab"));
    assert(!strcmp((char*)vector_get(v1, 1), "def"));

    vector *v2 = sstring_split(str1, 'a');
    assert(vector_size(v2) == 2);
    assert(!strcmp((char*)vector_get(v2, 0), ""));
    assert(!strcmp((char*)vector_get(v2, 1), "bcdef"));

    vector *v3 = sstring_split(str1, 'f');
    assert(vector_size(v3) == 2);
    assert(!strcmp((char*)vector_get(v3, 0), "abcde"));
    assert(!strcmp((char*)vector_get(v3, 1), ""));

    sstring *str3 = cstr_to_sstring("This is a {} day, {}!");
    sstring_substitute(str3, 18, "{}", "friend");
    char *tmpp = sstring_to_cstr(str3);
    assert(!strcmp("This is a {} day, friend!", tmpp));
    free(tmpp);

    sstring *str4 = cstr_to_sstring("1234567890");
    char* tmppp = sstring_slice(str4, 2, 5);
    assert(!strcmp(tmppp, "345"));
    free(tmppp);

    sstring_destroy(str1);
    sstring_destroy(str2);
    sstring_destroy(str3);
    sstring_destroy(str4);

    vector_destroy(v1);
    vector_destroy(v2);
    vector_destroy(v3);


    // int len = sstring_append(str1, str2); // len == 6
    // sstring_to_cstr(str1); // == "abcdef"
    return 0;
}
