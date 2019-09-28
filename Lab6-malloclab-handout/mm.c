/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Ke Dong",
    /* First member's email address */
    "kedong0810@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* my macros */
#define WSIZE 4
#define CHUNKSIZE (1 << 12) // 4kb, 1 page
/* pack a size and allocated bit into a word*/
#define PACK(size, alloc)  ((size)|(alloc)) 

/* read and write a word at address p */
#define GET(p)   (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
// bp should point to the first byte of the payload (allocated block) or "Next" (unallocated block)
#define HDRP(bp)  ( (char *)(bp) - WSIZE) // char takes 1 byte (8 bits) 
#define FTRP(bp)  ( ((char *)(bp) + GET_SIZE(HDRP(bp))) - 2 * WSIZE)

#define NEXT_BLKP(bp) ( (char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ( (char *)(bp) - GET_SIZE( (char *)(bp) - 2 * WSIZE  ) )

#define MAX(x, y) ((x) > (y) ? (x) : (y))
/* my own global scalar or pointers */
static void * heap_listp; // pointer to the root of the heap list

static int block_check(int verbose);
static void insert_heap_list(void* bp);
static void* extend_heap(size_t words);
static void* search_first(size_t words);
static void remove_block(void * bp);
static void* block_coalesce(void* bp);
static int heap_list_check(int verbose);

static int block_check(int verbose){
    // check the consistency of every block and print every block's information
    void * mem_start_brk = mem_heap_lo(); // the first byte of the heap

    if ( GET(mem_start_brk) != 0){// the first block is corrupted
        printf("The first block of the heap should be put 0 instead of %x \n", GET(mem_start_brk));
        return 0;
    }

    // check the root header
    void * heap_listp_current = mem_start_brk + 2 * WSIZE;
    if (heap_listp_current != heap_listp){
        printf("heap_listp dost not match \n");
        return 0;
    }
    if (verbose){
        printf("Index\t Address\t Size\t Occupied\n");
    }
    printf("0\t %x\t %d\t %d\n", heap_listp, GET_SIZE(heap_listp_current - WSIZE), 1);
   
    // check every block

    void * p = heap_listp_current + GET_SIZE(heap_listp_current - WSIZE);
    int counter = 1;
    size_t block_size = GET_SIZE(HDRP(p));
    while (block_size > 0){
        if (verbose){
            printf("%d\t %x\t %d\t %d\n", counter, p, GET_SIZE(FTRP(p)), GET_ALLOC(FTRP(p)));
        }
        
        if (GET(HDRP(p)) != GET(FTRP(p))){
            printf("header dose not equal to the footer\n");
            printf("footer size %d\n", GET_SIZE(FTRP(p)));
            return 0;
        }
        p = NEXT_BLKP(p);
        block_size = GET_SIZE(HDRP(p));
        counter += 1;
    }
    return 1;

}

static int heap_list_check(int verbose){
    void * bp = (void *)GET(heap_listp);
    size_t alloc = 0;
    int counter = 1;
    if (verbose)
        printf("index\t address\t size\t prev add\t next add\t \n");
    while (bp != NULL){
        if (verbose){
           printf("%d\t %x\t %d\t %x\t %x\t \n", counter, bp, GET_SIZE(HDRP(bp)), GET(bp+WSIZE), GET(bp));
        }
        alloc = GET_ALLOC(HDRP(bp));
        if (alloc){
            printf("Current free block is set to be occupied\n");
            return 0;
        }
        if (GET(HDRP(bp)) != GET(FTRP(bp))){
            printf("header dose not equal to the footer\n");
            printf("footer size %d\n", GET_SIZE(FTRP(bp)));
            return 0;
        }
        bp = (void *)GET(bp);
        counter += 1;
    }
    return 1;
}

static void insert_heap_list(void* bp){
    // insert the block in the heap list, assume the new block has been coalesced properly
    unsigned int old_next = GET(heap_listp); // the address of the previous next block
    PUT(heap_listp, (unsigned int)bp);
    PUT(bp, old_next);
    PUT(bp + WSIZE, (unsigned int)NULL);
    if (old_next != (unsigned int)NULL){
        PUT(old_next + WSIZE, (unsigned int)bp);
    }
    
} 

static void* extend_heap(size_t words){
    // extend the size of the heap by words bytes
    void * bp;
    size_t size;
    size = ALIGN(words); // round here

    bp = mem_sbrk(size); // allocate a new block
    if ((long)bp == -1)
        return NULL; // fail to allocate a new block
    
    // use the previous Epilogue header as the current header
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue header
    //printf("newly extend a block at %x with size %d\n", bp, size);
    bp = block_coalesce(bp);
    return bp;
}

static void* search_first(size_t words){
    size_t size;
    size = ALIGN(words);

    void * p = NULL;
    p = (void *)GET(heap_listp);
    while ( (p != NULL) && ( GET_SIZE(HDRP(p)) < size)){
        p = (void *)GET(p); // look for the first available block
    }

    if (p == NULL){
        return NULL;
    }

    return p;
}

static void remove_block(void * bp){
    void* prev_bp, * next_bp;
    // put the previous's next to the current block's next
    prev_bp = (void *)GET(bp + WSIZE); 
    if (prev_bp == NULL){
        prev_bp = heap_listp; // the previous 
    }
    PUT(prev_bp, GET(bp));

    // put the next's previous to the current block's previous
    next_bp = (void *)GET(bp);
    if ( next_bp != NULL){
        PUT(next_bp + WSIZE, (unsigned int)prev_bp);
    }
}

static void* block_coalesce(void* bp){
    // try to coalesce the current free block with neighboring blocks
    // assume that the current block has not been added into the free block list
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc){  /* Case 1 */
        ;
    }

    else if (prev_alloc && !next_alloc){ /* Case 2 */
        remove_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc){ /* Case 3 */
        remove_block(PREV_BLKP(bp));
        size += GET_SIZE(FTRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    else if (!prev_alloc && !next_alloc){ /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        remove_block(NEXT_BLKP(bp));
        remove_block(PREV_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

    }
    return bp;
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    mem_init(); // initialize the memory system
    heap_listp = mem_sbrk( 6 * WSIZE);
    if (heap_listp == (void *) -1)
        return -1; 
    // ask 6 words from the system
    PUT(heap_listp, 0); // alignment packing
    PUT(heap_listp + WSIZE, PACK(4*WSIZE, 1)); //root header
    PUT(heap_listp + 2 * WSIZE, (unsigned int)NULL); // the next block of the root is None
    PUT(heap_listp + 3 * WSIZE, (unsigned int)NULL); // the previous block of the root is None
    PUT(heap_listp + 4 * WSIZE, PACK(4*WSIZE, 1)); // root footer
    PUT(heap_listp + 5 * WSIZE, PACK(0, 1)); // Epilogue header
    heap_listp += 2 * WSIZE;
    void* bp = extend_heap(CHUNKSIZE);
    if (bp == NULL){
        return -1;
    }
    insert_heap_list(bp); // insert the newly generated block into the heap list
    //block_check(1);
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size){
   // printf("allocate a new block with size %ld\n", size);
    size_t newsize = ALIGN(size + 2 * WSIZE); 
    void* bp = search_first(newsize);
    //void* bp = heap_listp + 4 * WSIZE;
    if (bp == NULL){ // no feasible block to allocate the data
        bp = extend_heap(newsize); // no need to add this block into the free block list, since we are going to use it
        if (bp == NULL){
            printf("extend fails\n");
            return NULL;
        }
        insert_heap_list(bp);
        bp = search_first(newsize);
    }

    remove_block(bp); // remove the current block from the free block list
    size_t block_size = GET_SIZE(HDRP(bp));
    if ( block_size == newsize){
        PUT(HDRP(bp), PACK(newsize, 1));
        PUT(FTRP(bp), PACK(newsize, 1));
        return bp;              
    }else{
        // we need to split the block
        PUT(HDRP(bp), PACK(newsize, 1)); // since we change the size here, we correspondingly the position of the block footer
        PUT(FTRP(bp), PACK(newsize, 1));
        // set the remaining area to be a block
        void * newbp = FTRP(bp) + 2 * WSIZE;
        PUT(newbp - WSIZE, PACK(block_size - newsize, 0));
        PUT(FTRP(newbp), PACK(block_size - newsize, 0));
        insert_heap_list(newbp);
        return bp;
    }
    
    //block_check(1);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp){
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    bp = block_coalesce(bp);
    insert_heap_list(bp);
    return ;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize, next_alloc, newsize, aligned_size;
    aligned_size = ALIGN(size + 2 * WSIZE);
    if (size == 0){
        mm_free(oldptr);
        return oldptr;
    }

    if (ptr == NULL){
        newptr = mm_malloc(size);
        return newptr;
    }else{ // ptr is not NULL
        copySize = GET_SIZE(HDRP(ptr));
        if (size < copySize)
            copySize = size;
        if (aligned_size < GET_SIZE(HDRP(ptr))){// required size is smaller than current size, do nothing
            return ptr;
        }else{
            next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
            newsize = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
            if (!next_alloc && newsize >= aligned_size){
                remove_block(NEXT_BLKP(ptr));
                PUT(HDRP(ptr), PACK(newsize, 1));
                PUT(FTRP(ptr), PACK(newsize, 1));
                return ptr;
            }else{
                ptr = block_coalesce(ptr);
                if (GET_SIZE(HDRP(ptr)) >= aligned_size){
                    newptr = ptr;
                    memmove(newptr, oldptr, copySize);
                    PUT(HDRP(newptr), PACK(GET_SIZE(HDRP(ptr)), 1));
                    PUT(FTRP(newptr), PACK(GET_SIZE(HDRP(ptr)), 1));
                    return newptr;
                }else{
                    newptr = mm_malloc(size);
                    if (newptr == NULL)
                        return NULL;
                    memcpy(newptr, oldptr, copySize);
                    insert_heap_list(ptr);
                    return newptr;
                }
            }
        }
    }

    return newptr;
}














