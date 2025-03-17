#define USE_REAL_SBRK 0
#pragma GCC diagnostic ignored "-Wunused-function"

#if USE_REAL_SBRK
#define _GNU_SOURCE

#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#include "malloc.h"

/* Function to allocate heap. Do not modify.
 * This is a wrapper around the system call sbrk.
 * For initialization, you can call this function as allocHeap(NULL, size)
 *   -> It will allocate a heap of size <size> bytes and return a pointer to the start address
 * For enlarging the heap, you can later call allocHeap(heapaddress, newsize)
 *   -> heapaddress must be the address previously returned by allocHeap(NULL, size)
 *   -> newsize is the new size
 *   -> function will return NULL (no more memory available) or heapaddress (if ok)
 */

uint8_t *allocHeap(uint8_t *currentHeap, uint64_t size)
{               
        static uint64_t heapSize = 0;
        if( currentHeap == NULL ) {
                uint8_t *newHeap  = sbrk(size);
                if(newHeap)
                        heapSize = size;
                return newHeap;
        }
	uint8_t *newstart = sbrk(size - heapSize);
	if(newstart == NULL) return NULL;
	heapSize += size;
	return currentHeap;
}
#else
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "malloc.h"
// This is a "fake" version that you can use on MacOS
// sbrk as used above is not available on MacOS
// and normal malloc allocation does not easily allow resizing the allocated memory
uint8_t *allocHeap(uint8_t *currentHeap, uint64_t size)
{
        static uint64_t heapSize = 0;
        if( currentHeap == NULL ) {
                uint8_t *newHeap  = malloc(10*size);
                if(newHeap)
                        heapSize = 10*size;
                return newHeap;
        }
	if(size <= heapSize) return currentHeap;
        return NULL;
}
#endif


/* Global variable that indicates if debug is enabled or not */
int debug = 0;

/* Mutex for thread synchronization */
static pthread_mutex_t malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Allocation strategy */
static AllocType _currentStrategy = ALLOC_BESTFIT;

/* Last allocated block for next-fit strategy */
static Block *_lastAllocatedBlock = NULL;

/*
 * This is the heap you should use.
 * (Initialized with a heap of size HEAP_SIZE) in initAllocator())
 */
uint8_t *_heapStart = NULL;
uint64_t _heapSize = 0;

/*
 * This should point to the first free block in memory.
 */
Block *_firstFreeBlock;

/*
 * Initializes the memory block. You don't need to change this.
 */
void initAllocator()
{
    pthread_mutex_lock(&malloc_mutex);
    _heapStart = allocHeap(NULL, HEAP_SIZE);
    _heapSize = HEAP_SIZE;
    
    // Initialize first free block
    _firstFreeBlock = (Block*)_heapStart;
    _firstFreeBlock->size = HEAP_SIZE;
    _firstFreeBlock->next = NULL;
    pthread_mutex_unlock(&malloc_mutex);
}


/*
 * Gets the next block that should start after the current one.
 */
static Block *_getNextBlockBySize(const Block *current)
{
	(void)current;
	return NULL;
	// See lab tutorial
}

/*
 * Dumps the allocator. You should not need to modify this.
 */
void dumpAllocator()
{
    pthread_mutex_lock(&malloc_mutex);
    Block *current = _firstFreeBlock;
    printf("Free blocks:\n");
    while (current != NULL) {
        printf("  Block at %p: size %lu\n", (void*)current, current->size);
        current = current->next;
    }
    printf("\n");
    pthread_mutex_unlock(&malloc_mutex);
}

/*
 * Round the integer up to the block header size (16 Bytes).
 */
uint64_t roundUp(uint64_t n)
{
	// See lab tutorial
	return n;
}

/* Helper function that allocates a block 
 * takes as first argument the address of the next pointer that needs to be updated (!)
 */
static void *allocate_block(Block **update_next, Block *block, uint64_t new_size)
{
	(void)update_next;
	(void)block;
	(void)new_size;
	/* Not mandatory but possibly useful to implement this as a separate function
	 * called by my_malloc */
	return NULL;
}

void *my_malloc(uint64_t size)
{
    if (size == 0) return NULL;
    
    pthread_mutex_lock(&malloc_mutex);
    
    if (debug) {
        printf("Before allocation:\n");
        dumpAllocator();
    }
    
    uint64_t needed_size = roundUp(size + HEADER_SIZE);
    if (needed_size > HEAP_SIZE - HEADER_SIZE) {
        pthread_mutex_unlock(&malloc_mutex);
        return NULL;
    }
    
    Block *prev = NULL;
    Block *best_block = find_block(needed_size, &prev);
    
    if (best_block != NULL) {
        // Remove from free list
        if (prev == NULL) {
            _firstFreeBlock = best_block->next;
        } else {
            prev->next = best_block->next;
        }
        
        // Split if block is too big
        if (best_block->size > needed_size + HEADER_SIZE) {
            Block *new_block = (Block*)((uint8_t*)best_block + needed_size);
            new_block->size = best_block->size - needed_size;
            new_block->next = _firstFreeBlock;
            _firstFreeBlock = new_block;
            best_block->size = needed_size;
        }
        
        best_block->next = ALLOCATED_BLOCK_MAGIC;
        
        if (debug) {
            printf("After allocation:\n");
            dumpAllocator();
        }
        
        pthread_mutex_unlock(&malloc_mutex);
        return best_block->data;
    }
    
    // Try to extend heap
    uint64_t new_size = _heapSize + HEAP_SIZE;
    if (allocHeap(_heapStart, new_size) != NULL) {
        Block *new_block = (Block*)(_heapStart + _heapSize);
        new_block->size = HEAP_SIZE;
        new_block->next = _firstFreeBlock;
        _firstFreeBlock = new_block;
        _heapSize = new_size;
        pthread_mutex_unlock(&malloc_mutex);
        return my_malloc(size);
    }
    
    pthread_mutex_unlock(&malloc_mutex);
    return NULL;
}


/* Helper function to merge two freelist blocks.
 * Assume: block1 is at a lower address than block2
 * Does nothing if blocks are not neighbors (i.e. if block1 address + block1 size is not block2 address)
 * Otherwise, merges block by merging block2 into block1 (updates block1's size and next pointer
 */
static void merge_blocks(Block *block1, Block *block2)
{
	(void)block1;
	(void)block2;
	/* TODO: Implement */
	/* Note: Again this is not mandatory but possibly useful to put this in a separate
	 * function called by my_free */
}


void my_free(void *address)
{
    if (address == NULL) return;
    
    pthread_mutex_lock(&malloc_mutex);
    
    Block *block = (Block*)((uint8_t*)address - HEADER_SIZE);
    if (block->next != ALLOCATED_BLOCK_MAGIC) {
        pthread_mutex_unlock(&malloc_mutex);
        return;
    }
    
    // Remember original block for next-fit
    Block *original_block = block;
    
    // First, check if we can merge with next block
    Block *next_block = _getNextBlockBySize(block);
    if (next_block && next_block->next != ALLOCATED_BLOCK_MAGIC) {
        // Remove next block from free list
        Block **next_ptr = &_firstFreeBlock;
        while (*next_ptr != next_block) {
            next_ptr = &(*next_ptr)->next;
        }
        *next_ptr = next_block->next;
        block->size += next_block->size;
    }
    
    // Then check if we can merge with previous block
    Block *prev_block = _firstFreeBlock;
    Block **prev_ptr = &_firstFreeBlock;
    while (prev_block != NULL) {
        Block *next = _getNextBlockBySize(prev_block);
        if (next == block && prev_block->next != ALLOCATED_BLOCK_MAGIC) {
            // Merge with previous block
            prev_block->size += block->size;
            block = prev_block;  // Update block to point to merged block
            break;
        }
        prev_ptr = &prev_block->next;
        prev_block = *prev_ptr;
    }
    
    // If we didn't merge with a previous block, insert the block into free list
    if (prev_block == NULL) {
        Block **insert_ptr = &_firstFreeBlock;
        while (*insert_ptr != NULL && *insert_ptr < block) {
            insert_ptr = &(*insert_ptr)->next;
        }
        block->next = *insert_ptr;
        *insert_ptr = block;
    }
    
    // Update _lastAllocatedBlock for next-fit
    if (_currentStrategy == ALLOC_NEXTFIT && 
        (_lastAllocatedBlock == original_block || 
         _lastAllocatedBlock == next_block)) {
        _lastAllocatedBlock = block;
    }
    
    pthread_mutex_unlock(&malloc_mutex);
}

/* enable: 0 (false) to disable, 1 (true) to enable debug */
void enable_debug(int enable)
{
    pthread_mutex_lock(&malloc_mutex);
    debug = enable;
    pthread_mutex_unlock(&malloc_mutex);
}

