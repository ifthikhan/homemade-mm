/**
 * Unit tests for the memory allocator.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "mm.h"

static void test_mm_init() {

    assert(hmm_mm_init() == 0 && "Memory manager initialization failed");
}

static void test_mm_malloc_size_zero() {

    hmm_mm_init();
    char *ptr = (char *)hmm_mm_malloc(0);
    assert(ptr == NULL && "OMG!!!");
    assert(errno == EINVAL && "errno should be set to EINVAL");
}

static void test_mm_malloc() {

    hmm_mm_init();
    char *str = "Hello";
    char *ptr = (char *)hmm_mm_malloc(sizeof(char) * 12);
    strcpy(ptr, str);
    assert(strcmp(ptr, str) == 0 && "The value stored is corrupted");
}

static void test_mm_malloc_multi() {

    hmm_mm_init();
    char *str = "Hello";
    char *ptr = (char *)hmm_mm_malloc(sizeof(char) * 12);
    strcpy(ptr, str);

    int n = 13;
    int *ptr2 = (int *)hmm_mm_malloc(sizeof(int) * 2);
    *ptr2 = n;

    assert(strcmp(ptr, str) == 0 && "The value stored is corrupted");
    assert( *ptr2 == n && "The value stored is corrupted");
}

static void test_mm_free() {

    hmm_mm_init();
    char *str = "Hello";
    char *ptr = (char *)hmm_mm_malloc(sizeof(char) * 12);
    strcpy(ptr, str);

    hmm_mm_free((void *)ptr);
    ptr = (char *)hmm_mm_malloc(sizeof(char) * 12);
    char *str2 = "World";
    strcpy(ptr, str2);
    assert(strcmp(ptr, str2) == 0 && "The value stored is corrupted");
}

static void test_mm_free_multi() {

    hmm_mm_init();

    char *str = "Hello";
    char *ptr = (char *)hmm_mm_malloc(sizeof(char) * 12);
    strcpy(ptr, str);

    char *str2 = "World";
    char *ptr2 = (char *)hmm_mm_malloc(sizeof(char) * 12);
    strcpy(ptr2, str2);

    hmm_mm_free(ptr);
    hmm_mm_free(ptr2);
}

static void test_mm_max_heap() {

    int max_size = 2097152;
    char *ptr = (char *)hmm_mm_malloc(max_size);
    assert(ptr == NULL && "No protection if size specified is gt max size");
    assert(errno == ENOMEM && "When malloc fails ENOMEM should be set");
}

static void test_mm_calloc() {

    char *ptr = (char *)hmm_mm_calloc(10, sizeof(char));
    int i;
    for (i = 0; i < 10; i++, ptr++) {
        assert(*ptr == 0 && "The byte is not '0'");
    }
}

static void test_mm_calloc_nitems_zero() {

    char *ptr = (char *)hmm_mm_calloc(0, sizeof(char));
    assert(ptr == NULL && "if nitems is zero null should be returned");
    assert(errno == EINVAL && "errno should be set EINVAL");
}

static void test_mm_calloc_size_zero() {

    char *ptr = (char *)hmm_mm_calloc(10, 0);
    assert(ptr == NULL && "if size is zero null should be returned");
    assert(errno == EINVAL && "errno should be set EINVAL");
}

static void test_mm_calloc_nitems_zero_size_zero() {

    char *ptr = (char *)hmm_mm_calloc(0, 0);
    assert(ptr == NULL && "if nitems and size is zero null should be returned");
    assert(errno == EINVAL && "errno should be set EINVAL");
}

static void test_mm_realloc() {

    int n = sizeof(char) * 10;

    char *ptr = hmm_mm_malloc(n);
    memset(ptr, 15, n);

    char *ptr2;
    ptr2 = hmm_mm_realloc(ptr, n * 2);

    int i;
    for (i = 0; i < n; i++, ptr2++) {
        assert(*ptr2 == 15 && "The byte set is not equal to 15");
    }
}

static void test_mm_realloc_ptr_null() {

    int n = sizeof(char) * 10;
    char *ptr;
    ptr = (char *)hmm_mm_realloc(NULL, n);
    char *str = "Hello bro!";

    strcpy(ptr, str);
    assert(strcmp(ptr, str) == 0 && "When null ptr is given does not act as malloc");
}

static void test_mm_realloc_size_zero() {

    char *ptr;
    ptr = hmm_mm_malloc(10);
    ptr = hmm_mm_realloc(ptr, 0);
    assert(ptr == NULL && "If size is zero NULL should be returned");
}

static void test_mm_realloc_ptr_null_size_zero() {

    char *ptr = hmm_mm_realloc(NULL, 0);
    assert(ptr == NULL && "if ptr is null and size is zero null is returned");
}

int main() {

    test_mm_init();

    test_mm_malloc_size_zero();
    test_mm_malloc();
    test_mm_malloc_multi();

    test_mm_free();
    test_mm_free_multi();

    test_mm_max_heap();

    test_mm_calloc();
    test_mm_calloc_nitems_zero();
    test_mm_calloc_size_zero();
    test_mm_calloc_nitems_zero_size_zero();

    test_mm_realloc();
    test_mm_realloc_ptr_null();
    test_mm_realloc_size_zero();
    test_mm_realloc_ptr_null_size_zero();

    return EXIT_SUCCESS;
}
