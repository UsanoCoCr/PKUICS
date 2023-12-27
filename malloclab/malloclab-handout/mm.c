/*
/*
 * mm.c
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#else
#define dbg_printf(...)
#endif

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* Basic constants and macros */
#define WSIZE 4             /* Word and header/footer size (bytes) */
#define DSIZE 8             /* Double word size (bytes) */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) */
#define PLACE_DIVIDE 128    /* Dicide whether to put the block to head or foot */
#define SEG_LEN 12          /* Number of lists */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define GETL(p) (*(unsigned long *)(p))
#define PUTL(p, val) (*(unsigned long *)(p) = (val))
#define SET_PTR(p, ptr) (*(unsigned long *)(p) = (unsigned long)(uintptr_t)(ptr))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer, and its Pred and Succ */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define PRDP(bp) ((char *)(bp))
#define SUCP(bp) ((char *)(bp) + DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))
#define PRED_BLKP(bp) ((char *)(uintptr_t)GETL(PRDP(bp)))
#define SUCC_BLKP(bp) ((char *)(uintptr_t)GETL(SUCP(bp)))

/* Global variables */
static char *heap_listp = 0;                /* Pointer to first block */
static unsigned long *free_list_ptr = NULL; /* Pointers to each free block list's starter */

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void *place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void *put_to_list(void *bp);
static void delete_from_list(void *bp);
static int get_index(size_t bsize);
static unsigned long *free_list(int idx);

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT - 1)) & ~0x7)

/*
 * Initialize: return -1 on error, 0 on success.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4 * WSIZE + (SEG_LEN + 1) * DSIZE)) == (void *)-1)
        return -1;

    // list init
    free_list_ptr = (unsigned long *)heap_listp;
    for (int i = 0; i < SEG_LEN + 1; i++)
    {
        SET_PTR(free_list(i), 0);
    }
    heap_listp += ((SEG_LEN + 1) * DSIZE);

    PUT(heap_listp, 0);                            /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2 * WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

/*
 * malloc
 */
void *malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;

    if (heap_listp == 0)
    {
        mm_init();
    }
    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= 2 * DSIZE)
        asize = 3 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE - 1)) / DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL)
    {
        return place(bp, asize);
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    return place(bp, asize);
}

/*
 * free
 */
void free(void *ptr)
{
    if (!ptr)
        return;
}

/*
 * realloc - you may want to look at mm-naive.c
 */
void *realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void *newptr;

    // If size == 0 then this is just free, and we return NULL.
    if (size == 0)
    {
        free(ptr);
        return 0;
    }

    // If oldptr is NULL, then this is just malloc.
    if (ptr == NULL)
    {
        return malloc(size);
    }

    newptr = malloc(size);

    // If realloc() fails the original block is left untouched
    if (!newptr)
    {
        return 0;
    }

    // Copy the old data.
    oldsize = GET_SIZE(HDRP(ptr));
    if (size < oldsize)
        oldsize = size;
    memcpy(newptr, ptr, oldsize);

    // Free the old block.
    free(ptr);

    return newptr;
}

/*
 * calloc - you may want to look at mm-naive.c
 * This function is not tested by mdriver, but it is
 * needed to run the traces.
 */
void *calloc(size_t nmemb, size_t size)
{
    return NULL;
}

/*
 * Return whether the pointer is in the heap.
 * May be useful for debugging.
 */
static int in_heap(const void *p)
{
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}

/*
 * Return whether the pointer is aligned.
 * May be useful for debugging.
 */
static int aligned(const void *p)
{
    return (size_t)ALIGN(p) == (size_t)p;
}

/*
 * mm_checkheap
 */
void mm_checkheap(int lineno)
{
}

//
/*helpers*/
//

/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = (char *)mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0)); /* Free block header */
    PUT(FTRP(bp), PACK(size, 0)); /* Free block footer */
    put_to_list(bp);
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * get_index - sort a free block according to its size;
 * or find a new block should be allocated from which list
 */
static int get_index(size_t bsize)
{
    int idx = 0;
    long scale = 32;
    for (; idx < SEG_LEN; idx++)
    {
        if (bsize <= (size_t)scale)
            return idx;
        scale *= 2;
    }
    return SEG_LEN;
}

/*
 * free_list - get the ptr of each list
 */
static unsigned long *free_list(int idx)
{
    return (unsigned long *)((char *)free_list_ptr + (DSIZE * idx));
}
/*
 * put_to_list - put a new free block to its list
 */
static void *put_to_list(void *bp)
{
    int index = get_index(GET_SIZE(HDRP(bp)));
    unsigned long *prev_end = (unsigned long *)(uintptr_t)GETL(free_list(index));

    if (prev_end != 0)
    { // not an empty list
        SET_PTR(SUCP(prev_end), bp);
        SET_PTR(PRDP(bp), prev_end);
        SET_PTR(SUCP(bp), 0);
        SET_PTR(free_list(index), bp);
    }
    else
    { // empty list
        SET_PTR(free_list(index), bp);
        SET_PTR(PRDP(bp), 0);
        SET_PTR(SUCP(bp), 0);
    }

    return bp;
}

/*
 * delete_from_list - delete a free block from its list
 */
static void delete_from_list(void *bp)
{
    int index = get_index(GET_SIZE(HDRP(bp)));

    if (PRED_BLKP(bp) == 0)
    { // bp is start block
        if (SUCC_BLKP(bp) == 0)
        { // is an end block
            SET_PTR(free_list(index), 0);
        }
        else
        {
            SET_PTR(PRDP(SUCC_BLKP(bp)), 0);
        }
    }

    else
    { // not a start block
        if (SUCC_BLKP(bp) == 0)
        { // is an end block
            SET_PTR(free_list(index), PRED_BLKP(bp));
            SET_PTR(SUCP(PRED_BLKP(bp)), 0);
        }
        else
        {
            SET_PTR(SUCP(PRED_BLKP(bp)), SUCC_BLKP(bp));
            SET_PTR(PRDP(SUCC_BLKP(bp)), PRED_BLKP(bp));
        }
    }
}

/*
 * coalesce - put nearby free blocks together, and refresh the lists
 * this function has another effect: maintain the lists
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)
    { /* Case 1: lonely free block */
        return bp;
    }

    else if (prev_alloc && !next_alloc)
    { /* Case 2: next block is free */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_from_list(NEXT_BLKP(bp));
        delete_from_list(bp);

        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc)
    { /* Case 3: prev block is free */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        delete_from_list(PREV_BLKP(bp));
        delete_from_list(bp);

        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    else
    { /* Case 4: both prev and next are free */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        delete_from_list(PREV_BLKP(bp));
        delete_from_list(NEXT_BLKP(bp));
        delete_from_list(bp);

        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    put_to_list(bp);
    return bp;
}

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
static void *place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    size_t rsize = csize - asize;
    delete_from_list(bp);
    if (rsize >= (3 * DSIZE))
    { // separable
        if (asize > PLACE_DIVIDE)
        { // big block, put to foot
            PUT(HDRP(bp), PACK(rsize, 0));
            PUT(FTRP(bp), PACK(rsize, 0));
            PUT(HDRP(NEXT_BLKP(bp)), PACK(asize, 1));
            PUT(FTRP(NEXT_BLKP(bp)), PACK(asize, 1));
            put_to_list(bp);
            return NEXT_BLKP(bp);
        }
        else
        { // small block, put to head
            PUT(HDRP(bp), PACK(asize, 1));
            PUT(FTRP(bp), PACK(asize, 1));
            PUT(HDRP(NEXT_BLKP(bp)), PACK(rsize, 0));
            PUT(FTRP(NEXT_BLKP(bp)), PACK(rsize, 0));
            put_to_list(NEXT_BLKP(bp));
        }
    }

    else
    { // not separable
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
    return bp;
}

/*
 * find_fit - find which free block could be used for allocation.
 * strategy: first fit in each list; look for a capable free block from the smallest fit list by LIFO
 */
static void *find_fit(size_t asize)
{
    int idx = get_index(asize);

    while (idx <= SEG_LEN)
    {
        // empty list
        if (GETL(free_list(idx)) == 0)
        {
            idx++;
            continue;
        }
        // LIFO: find from the last block in the list
        char *bp = (char *)(uintptr_t)GETL(free_list(idx));

        while (1)
        {
            if (GET_SIZE(HDRP(bp)) >= (unsigned int)asize)
                return bp;
            bp = (char *)PRED_BLKP(bp);

            if (bp == 0)
                break;
        }
        // if not found, go to a list with bigger blocks
        idx++;
    }

    return NULL;
}