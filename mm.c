#include <stdio.h>
#include <stdlib.h>
#include <errorno.h>
#include <sys/mman.h>

#include "memlib.h"
#include "mm.h"


// Header and footer sizes (bytes).
#define WSIZE   4

// Double word sized (bytes).
#define DSIZE  8

#define MAX(x, y)   (((x) > (y)) ? (x) : (y))

// Extend the heap by the following size (4KB)
#define CHUNKSIZE (1<<12)

//Pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))

//Read and write a word at address p
#define GET(p)          (*(unsigned int *)p)
#define PUT(p, val)     (*(unsigned int *)p = (val))

#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

// bp signifies block pointer and they are usually addresses to
// the data blocks. (Convention :))
#define HDRP(bp)    ((char *)(bp) - WSIZE)
#define FTRP(bp)    ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))


static char* heap_listp = 0;


static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);


int hmm_mm_init(void) {

    /*
     * Allocate a 16 bytes in the heap.
     * 04 bytes     Offset
     * 16 bytes     Prologue
     * 04 bytes     Epilogue
     */
    heap_listp = hmm_mem_sbrk(WSIZE * 4);
    if (heap_listp == (void *)-1)
        return -1;

    /*
     * Allocation block conventions. The first byte is set to zero and used
     * as an alignment. The next 12 bytes are the prologue and epilogue
     * areas, this used to define the boundaries of the heap space.
     */
    PUT(heap_listp, 0); // Alignment padding
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); //Prologue header
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); //Prologue footer
    PUT(heap_listp + (3*WSIZE), PACK(0, 1)); //Epilogue header

    // Move the base pointer to point to the prologue footer.
    heap_listp += (2*WSIZE);

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;

    return 0;
}

/*
 * Allocate a block in memory with atleast size bytes for the payload. If
 * sufficient space is not found the function will return NULL
 */
void *hm_mm_malloc(size_t size) {

    if (heap_listp == 0)
        hmm_mm_init();

    if (size == 0)
        return NULL;

    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;

    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else {
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
        // The below should also work
        //asize = (size - (size%DSIZE)) + (2 * DSIZE)
    }

    // Search the free list for a fit.
    bp =  find_fit(asize);
    if (bp != NULL) {
        place(bp, asize);
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    bp = extend_heap(extendsize/WSIZE);
    if (bp  == NULL)
        return NULL;

    place(bp, asize);
    return bp;
}

/**
 * Free the given block
 */
void hmm_mm_free(void *bp) {

    if (bp == 0)
        return;

    size_t size = GET_SIZE(HDRP(bp));

    if (heap_listp == 0){
        mm_init();
    }

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * Given a block pointer try to coalesce the previous and next
 * blocks if they are empty
 */
static void *coalesce(void *bp) {

    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (next_alloc && prev_alloc) {
        return bp;
    }
    else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    return bp;
}

static void *extend_heap(size_t words) {

    size_t size;
    size = (words & 1) ? (words + 1) : words;
    size = size * WSIZE;

    char *bp;
    bp = hmm_mem_sbrk(size);
    if (*bp == (void *)-1)
        return NULL;

    // The new block header and footer
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    // Epilogue header
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

/**
 * Find a suitable free block. Currently uses a "first fit" approach.
 */
static void *find_fit(size_t asize) {

    char *bp = heap_listp;
    while(GET_SIZE(HDRP(bp)) != 0) {
        if (!GET_ALLOC(HDRP(bp)))
            return bp;

        bp = NEXT_BLKP(bp);
    }

    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= asize)
            return bp

    return NULL;
}

static void place(void *bp, size_t asize) {

    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (2*DSIZE)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
    }
    else{
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}
