/*
 * mm.c
 * 康子熙 2200017416
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 *
 * 我实现了一个基于分离空闲链表的动态内存分配器。使用的策略是FIFO，同时进行的指针存储的优化和取脚部
 * 同时，我还在放置的时候实现了一个分类讨论，将大小相近的块放在一起，这样可以减小外部碎片
 * 在扩大堆时，我也进行了判断，来保证空间的利用率达到最大值
 *
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
#define CHUNKSIZE (3072)
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define PATIANT 10

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
static char *heap_listp = 0; /* Pointer to first block */
static char *array_start = 0;
static char *array_end = 0;
static char *chunkend = 0;

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void *place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void insert(void *bp);
static void erase(void *bp);
static size_t get_index(size_t size);
static void *PRED(void *bp);
static void *SUCC(void *bp);
static void SET_PRED(void *bp, void *pred);
static void SET_SUCC(void *bp, void *succ);
static unsigned int GET_OFFSET(void *bp);
static void *get_array_start(size_t index);
static void place_array_start(size_t index, void *bp);
static void *get_array_end(size_t index);
static void place_array_end(size_t index, void *bp);

/*
 * Initialize: return -1 on error, 0 on success.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(34 * WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + WSIZE, 0);
    PUT(heap_listp + 2 * WSIZE, 0);
    PUT(heap_listp + 3 * WSIZE, 0);
    PUT(heap_listp + 4 * WSIZE, 0);
    PUT(heap_listp + 5 * WSIZE, 0);
    PUT(heap_listp + 6 * WSIZE, 0);
    PUT(heap_listp + 7 * WSIZE, 0);
    PUT(heap_listp + 8 * WSIZE, 0);
    PUT(heap_listp + 9 * WSIZE, 0);
    PUT(heap_listp + 10 * WSIZE, 0);
    PUT(heap_listp + 11 * WSIZE, 0);
    PUT(heap_listp + 12 * WSIZE, 0);
    PUT(heap_listp + 13 * WSIZE, 0);
    PUT(heap_listp + 14 * WSIZE, 0);
    array_start = heap_listp;
    heap_listp += (15 * WSIZE);

    PUT(heap_listp + WSIZE, 0);
    PUT(heap_listp + 2 * WSIZE, 0);
    PUT(heap_listp + 3 * WSIZE, 0);
    PUT(heap_listp + 4 * WSIZE, 0);
    PUT(heap_listp + 5 * WSIZE, 0);
    PUT(heap_listp + 6 * WSIZE, 0);
    PUT(heap_listp + 7 * WSIZE, 0);
    PUT(heap_listp + 8 * WSIZE, 0);
    PUT(heap_listp + 9 * WSIZE, 0);
    PUT(heap_listp + 10 * WSIZE, 0);
    PUT(heap_listp + 11 * WSIZE, 0);
    PUT(heap_listp + 12 * WSIZE, 0);
    PUT(heap_listp + 13 * WSIZE, 0);
    PUT(heap_listp + 14 * WSIZE, 0);
    array_end = heap_listp;
    heap_listp += (15 * WSIZE);

    PUT(heap_listp, 0);                            /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 3)); /* Prologue header */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 3)); /* Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 3));     /* Epilogue header */
    chunkend = heap_listp + (3 * WSIZE);
    heap_listp += (2 * WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE >> 1) == NULL)
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
    char *bp = heap_listp;

    // Check prologue block
    if (GET_SIZE(HDRP(heap_listp)) != DSIZE || !GET_ALLOC(HDRP(heap_listp)))
    {
        printf("Error: bad prologue header at line: %d\n", lineno);
        exit(1);
    }

    // Check all blocks for conditions
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        // Check each block’s address alignment and heap boundaries
        if (!in_heap(bp) || !aligned(bp))
        {
            printf("Error: bad block address at line: %d\n", lineno);
            exit(1);
        }

        // Check each block’s header and footer
        if (GET(HDRP(bp)) != GET(FTRP(bp)))
        {
            printf("Error: header does not match footer at line: %d\n", lineno);
            exit(1);
        }
    }

    // Check each free block in the free list
    for (int i = 0; i < 15; i++)
    {
        void *bp = get_array_start(i);
        while (bp != NULL)
        {
            if (GET_ALLOC(HDRP(bp)))
            {
                printf("Error: free block in the allocated list at line: %d\n", lineno);
                exit(1);
            }
            bp = SUCC(bp);
        }
    }

    // Check coalescing: no two consecutive free blocks in the heap
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && !GET_ALLOC(HDRP(NEXT_BLKP(bp))))
        {
            printf("Error: two consecutive free blocks in the heap at line: %d\n", lineno);
            exit(1);
        }
    }
}

/*
 * extend_heap - 扩大堆
 */
static void *extend_heap(size_t words)
{
    char *temp = chunkend;
    size_t available = 0;
    while (1)
    {
        if (PREV_ALLOC(HDRP(temp)))
            break;
        temp = PREV_BLKP(temp);
        available += GET_SIZE(HDRP(temp));
    }

    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    size = MAX(CHUNKSIZE, size - available);
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0) | PREV_ALLOC(HDRP(bp))); /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));                        /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));                /* New epilogue header */
    chunkend = NEXT_BLKP(bp);

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
static void *place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    erase(bp);
    if ((csize - asize) >= (2 * DSIZE)) // 当剩余够存头+尾+prev+succ=16个字节时，可以分割
    {
        if (asize <= 96)
        {
            PUT(HDRP(bp), PACK(csize - asize, 0) | PREV_ALLOC(HDRP(bp)));
            PUT(FTRP(bp), PACK(csize - asize, 0));
            insert(bp);
            bp = NEXT_BLKP(bp);
            PUT(HDRP(bp), PACK(asize, 1));
            PUT(HDRP(NEXT_BLKP(bp)), GET(HDRP(NEXT_BLKP(bp))) | 0x2);
            return bp;
        }
        else
        {
            PUT(HDRP(bp), PACK(asize, 1) | PREV_ALLOC(HDRP(bp)));
            void *nextbp = NEXT_BLKP(bp);
            PUT(HDRP(nextbp), PACK(csize - asize, 2));
            PUT(FTRP(nextbp), PACK(csize - asize, 0));
            insert(nextbp);
            return bp;
        }
    }
    else
    {
        PUT(HDRP(bp), PACK(csize, 1) | PREV_ALLOC(HDRP(bp)));
        PUT(HDRP(NEXT_BLKP(bp)), GET(HDRP(NEXT_BLKP(bp))) | 0x2);
    }
    return bp;
}

/*
 * find_fit - Find a fit for a block with asize bytes
 */
static void *find_fit(size_t asize)
{
    int index = get_index(asize);
    for (int i = 0; i < PATIANT; i++)
    {
        void *bp = get_array_start(index);
        if (bp == NULL)
            break;
        if (GET_SIZE(HDRP(bp)) == asize)
            return bp;
        bp = SUCC(bp);
    }
    for (int i = index; i < 15; i++)
    {
        void *bp = get_array_start(i);
        while (bp != NULL)
        {
            if (GET_SIZE(HDRP(bp)) >= asize)
                return bp;
            bp = SUCC(bp);
        }
    }
    return NULL;
}

static void insert(void *bp)
{
    int size = GET_SIZE(HDRP(bp));
    int index = get_index(size);
    void *start = get_array_start(index);
    if (start == NULL)
    {
        place_array_start(index, bp);
        place_array_end(index, bp);
        SET_PRED(bp, NULL);
        SET_SUCC(bp, NULL);
        return;
    }
    void *end = get_array_end(index);
    SET_SUCC(end, bp);
    SET_PRED(bp, end);
    SET_SUCC(bp, NULL);
    place_array_end(index, bp);
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
            place_array_end(index, pred);
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
            PUT(array_start + index * WSIZE, 0);
            PUT(array_end + index * WSIZE, 0);
            return;
        }
        SET_PRED(succ, NULL);
        place_array_start(index, succ);
    }
    return;
}

static size_t get_index(size_t size)
{
    if (size <= 20)
        return 0;
    if (size <= 32)
        return 1;
    if (size <= 64)
        return 2;
    if (size == 128)
        return 3;
    if (size <= 256)
        return 4;
    if (size <= 512)
        return 5;
    if (size <= 1024)
        return 6;
    if (size <= 2048)
        return 7;
    if (size <= 4096)
        return 8;
    if (size <= 8192)
        return 9;
    if (size <= 16384)
        return 10;
    if (size <= 20480)
        return 11;
    if (size <= 24576)
        return 12;
    if (size <= 28672)
        return 13;
    return 14;
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

static void *get_array_start(size_t index)
{
    void *id_start = array_start + index * WSIZE;
    unsigned int offset = *(unsigned int *)(id_start);
    if (offset == 0)
        return NULL;
    return heap_listp + offset;
}
static void place_array_start(size_t index, void *bp)
{
    void *id_start = array_start + index * WSIZE;
    unsigned int offset = (unsigned int)((unsigned long)(bp) - (unsigned long)(heap_listp));
    PUT(id_start, offset);
}
static void *get_array_end(size_t index)
{
    void *id_end = array_end + index * WSIZE;
    unsigned int offset = *(unsigned int *)(id_end);
    if (offset == 0)
        return NULL;
    return heap_listp + offset;
}
static void place_array_end(size_t index, void *bp)
{
    void *id_end = array_end + index * WSIZE;
    unsigned int offset = (unsigned int)((unsigned long)(bp) - (unsigned long)(heap_listp));
    PUT(id_end, offset);
}