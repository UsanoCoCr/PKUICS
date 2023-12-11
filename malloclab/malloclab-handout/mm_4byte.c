/*
 * mm.c
 * 康子熙 2200017416
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT - 1)) & ~0x7)

/* define some useful micro */
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 11)
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* some basic strategy */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define PREV_ALLOC(p) (GET(p) & 0x2)
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))
#define SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))
#define SET_LOW_PRED(p, ptr) (SET_PTR((char *)(p), ptr))
#define SET_LOW_SUCC(p, ptr) (SET_PTR(((char *)(p) + (WSIZE)), ptr))

/* Global variables */
static char *heap_listp = 0;       /* Pointer to first block */
static char *free_start[17] = {0}; /* begin of the free list */
static char *free_tail[17] = {0};  /* tail of the free list */

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void insert(void *bp);
static void erase(void *bp);
static int get_index(size_t size);
static void *PRED(void *bp);
static void *SUCC(void *bp);
static void SET_PRED(void *bp, void *pred);
static void SET_SUCC(void *bp, void *succ);
static unsigned int GET_OFFSET(void *bp);

/*
 * Initialize: return -1 on error, 0 on success.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(2 * DSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                            /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 3));     /* Epilogue header */
    heap_listp += (2 * WSIZE);

    for (int i = 0; i < 17; i++)
    {
        free_start[i] = NULL;
        free_tail[i] = NULL;
    }

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
    if (size <= DSIZE + WSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (WSIZE) + (DSIZE - 1)) / DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * free
 */
void free(void *ptr)
{
    if (ptr == 0)
        return;

    size_t size = GET_SIZE(HDRP(ptr));
    if (heap_listp == 0)
    {
        mm_init();
    }

    PUT(HDRP(ptr), PACK(size, 0) | PREV_ALLOC(HDRP(ptr)));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * realloc - you may want to look at mm-naive.c
 */
void *realloc(void *oldptr, size_t size)
{
    size_t oldsize;
    void *newptr;
    size_t asize;

    /* If size == 0 then this is just free, and we return NULL. */
    if (size == 0)
    {
        free(oldptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if (oldptr == NULL)
        return malloc(size);

    oldsize = GET_SIZE(HDRP(oldptr));
    if (size <= DSIZE + WSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (WSIZE) + (DSIZE - 1)) / DSIZE);

    if (asize == oldsize)
    {
        return oldptr;
    }
    else if (asize < oldsize)
    {
        if (oldsize - asize >= 2 * DSIZE)
        {
            PUT(HDRP(oldptr), PACK(asize, 1) | PREV_ALLOC(HDRP(oldptr)));
            newptr = NEXT_BLKP(oldptr);
            PUT(HDRP(newptr), PACK(oldsize - asize, 2));
            PUT(FTRP(newptr), PACK(oldsize - asize, 0));
            insert(newptr);
        }
        return oldptr;
    }
    newptr = malloc(size);
    if (!newptr)
        return 0;
    memcpy(newptr, oldptr, oldsize);
    free(oldptr);
    return newptr;
}

/*
 * calloc - you may want to look at mm-naive.c
 * This function is not tested by mdriver, but it is
 * needed to run the traces.
 */
void *calloc(size_t nmemb, size_t size)
{
    size_t bytes = nmemb * size;
    void *newptr;

    newptr = malloc(bytes);
    memset(newptr, 0, bytes);

    return newptr;
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

/*
 * extend_heap - 扩大堆
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0) | PREV_ALLOC(HDRP(bp))); /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));                        /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));                /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = PREV_ALLOC(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)
    { /* Case 1 */
        PUT(HDRP(NEXT_BLKP(bp)), GET(HDRP(NEXT_BLKP(bp))) & ~0x2);
        insert(bp);
        return bp;
    }

    else if (prev_alloc && !next_alloc)
    { /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        erase(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0) | 0x2);
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc)
    { /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        erase(PREV_BLKP(bp));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0) | PREV_ALLOC(HDRP(PREV_BLKP(bp))));
        bp = PREV_BLKP(bp);
    }

    else
    { /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        erase(PREV_BLKP(bp));
        erase(NEXT_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0) | PREV_ALLOC(HDRP(PREV_BLKP(bp))));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    PUT(HDRP(NEXT_BLKP(bp)), GET(HDRP(NEXT_BLKP(bp))) & ~0x2);
    insert(bp);
    return bp;
}

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    erase(bp);

    if ((csize - asize) >= (2 * DSIZE)) // 当剩余够存头+尾+prev+succ=16个字节时，可以分割
    {
        PUT(HDRP(bp), PACK(asize, 1) | PREV_ALLOC(HDRP(bp)));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 2));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        insert(bp);
    }
    else
    {
        PUT(HDRP(bp), PACK(csize, 1) | PREV_ALLOC(HDRP(bp)));
        PUT(HDRP(NEXT_BLKP(bp)), GET(HDRP(NEXT_BLKP(bp))) | 0x2);
    }
}

/*
 * find_fit - Find a fit for a block with asize bytes
 */
static void *find_fit(size_t asize)
{
    int index = get_index(asize);
    for (int i = index; i < 17; i++)
    {
        void *temp = free_tail[i];
        for (; temp != NULL; temp = PRED(temp))
        {
            if (GET_SIZE(HDRP(temp)) >= asize)
                return temp;
        }
    }

    return NULL;
}

static void insert(void *bp)
{
    int index = get_index(GET_SIZE(HDRP(bp)));
    if (free_start[index] == NULL)
    {
        free_start[index] = bp;
        free_tail[index] = bp;
        SET_PRED(bp, NULL);
        SET_SUCC(bp, NULL);
        return;
    }
    void *old_tail = free_tail[index];
    SET_SUCC(old_tail, bp);
    SET_PRED(bp, old_tail);
    SET_SUCC(bp, NULL);
    free_tail[index] = bp;
    return;
}

static void erase(void *bp)
{
    int index = get_index(GET_SIZE(HDRP(bp)));
    void *pred = PRED(bp);
    void *succ = SUCC(bp);
    if (pred != NULL)
    {
        SET_SUCC(pred, succ);
        if (succ == NULL)
        {
            free_tail[index] = pred;
        }
        else
        {
            SET_PRED(succ, pred);
        }
    }
    else
    {
        if (succ == NULL)
        {
            free_tail[index] = NULL;
            free_start[index] = NULL;
            return;
        }
        SET_PRED(succ, NULL);
        free_start[index] = succ;
    }
    return;
}

static int get_index(size_t size)
{
    if (size <= 16)
        return 0;
    if (size <= 24)
        return 1;
    if (size <= 32)
        return 2;
    if (size <= 48)
        return 3;
    if (size <= 64)
        return 4;
    if (size <= 96)
        return 5;
    if (size <= 128)
        return 6;
    if (size <= 256)
        return 7;
    if (size <= 512)
        return 8;
    if (size <= 1024)
        return 9;
    if (size <= 2048)
        return 10;
    if (size <= 4096)
        return 11;
    if (size <= 8192)
        return 12;
    if (size <= 16384)
        return 13;
    if (size <= 32768)
        return 14;
    if (size <= 65536)
        return 15;
    return 16;
}

static void *PRED(void *bp)
{
    unsigned int offset = *(unsigned int *)(bp);
    if (offset == 0)
        return NULL;
    return heap_listp + offset;
}
static void *SUCC(void *bp)
{
    unsigned int offset = *(unsigned int *)((char *)(bp) + WSIZE);
    if (offset == 0)
        return NULL;
    return heap_listp + offset;
}
static unsigned int GET_OFFSET(void *bp)
{
    if (bp == NULL)
        return 0;
    char *temp = (char *)(bp);
    unsigned int offset = (unsigned int)((unsigned long)(temp) - (unsigned long)(heap_listp));
    return offset;
}
static void SET_PRED(void *bp, void *pred)
{
    SET_LOW_PRED(bp, GET_OFFSET(pred));
}
static void SET_SUCC(void *bp, void *succ)
{
    SET_LOW_SUCC(bp, GET_OFFSET(succ));
}