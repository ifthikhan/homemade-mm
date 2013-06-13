/*
 *  memlib.c - The memory management system module.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "memlib.h"

// Defining the MAX_HEAP_SIZE as 2MB
#define MAX_HEAP    2097152

// Points to first byte of heap
static char *mem_heap;

// Points to last byte of heap plus 1
static char *mem_brk;

//Points to max heap legal address plus 1
static char *mem_max_address;


/**
 * Initialize the memory system
 */
void hmm_mem_init(void) {

    mem_heap = (char *) mmap(NULL, MAX_HEAP, PROT_READ|PROT_WRITE, MAP_ANON,
                             NULL, 0);
    mem_brk = mem_heap;
    mem_max_address = (char *)(mem_heap + MAX_HEAP);
}

/**
 * sbrk to increase the size of the heap. If successful returns the address
 * of the previous sbrk value. Currently decreasing the heap space is not
 * supported.
 */
void *hmm_mem_sbrk(int incr) {

    char *old_brk = mem_brk;

    if (incr < 0 || (mem_brk + incr) > mem_max_address) {
        errno = ENOMEM;
        fprintf(stderr, "ERROR: hmm_mem_sbrk failed. Ran out of memory \n");
        return (void *)-1;
    }

    mem_brk += incr;
    return (void *)old_brk;
}

/*
 * Free the storage used by the memory system model.
 */
void hmm_mem_deinit(void) {
    // TODO: Implement the freeing of heap space.
}

/*
 * Reset the break pointer of the heap space.
 */
void hmm_mem_reset_brk() {

    mem_brk = (char *)mem_heap;
}

/*
 * Return the address of the first byte of the heap
 */
void *hmm_mem_heap_lo() {

    return (void *)mem_heap;
}

/*
 * Return the last address of the heap byte
 */
void * hmm_mem_heap_hi() {

    return (void *)(mem_brk - 1);
}

/*
 * Return the size of the heap
 */
size_t hmm_mem_heap_size() {

    return (size_t)((void *)mem_brk - (void *)mem_heap);
}

/*
 * Returns the size of the page.
 */
size_t mem_pagesize() {

    return (size_t)getpagesize();
}
