#include "umalloc.h"
#include "csbrk.h"
#include "ansicolors.h"
#include <stdio.h>
#include <assert.h>

const char author[] = ANSI_BOLD ANSI_COLOR_RED "BRYCE RICHARDSON" ANSI_RESET;

/*
 * The following helpers can be used to interact with the memory_block_t
 * struct, they can be adjusted as necessary.
 */
// A pointer to the first block in the heap (in memory order)
memory_block_t *free_head; 

/*
 * is_allocated - returns true if a block is marked as allocated.
 */
bool is_allocated(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & 0x1;
}

/*
 * allocate - marks a block as allocated.
 * design decision - null out the "next" field
 */
void allocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc |= 0x1;
    block->next = NULL;
}


/*
 * deallocate - marks a block as unallocated.
 */
void deallocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc &= ~0x1;
}

/*
 * get_size - gets the size of the block.
 */
size_t get_size(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & ~(ALIGNMENT-1);
}

/*
 * get_next - gets the next block.
 */
memory_block_t *get_next(memory_block_t *block) {
    if (block == NULL) {
        return NULL;
    }
    return block->next;
}

/*
 * put_block - puts a block struct into memory at the specified address.
 * Initializes the size and allocated fields, along with NUlling out the next 
 * field.
 * design decision - have put_block return 0 if successful and -1 otherwise  
 */
void put_block(memory_block_t *block, size_t size, bool alloc) {
    assert(block != NULL);
    assert(size % ALIGNMENT == 0);
    assert(alloc >> 1 == 0);
    block->block_size_alloc = size | alloc;
    block->next = NULL;
}

/*
 * get_payload - gets the payload of the block.
 */
void *get_payload(memory_block_t *block) {
    assert(block != NULL);
    return (void*)(block + 1);
}

/*
 * get_block - given a payload, returns the block.
 */
memory_block_t *get_block(void *payload) {
    assert(payload != NULL);
    return ((memory_block_t *)payload) - 1;
}

/*
 * The following are helper functions that can be implemented to assist in your
 * design, but they are not required. 
 */

 /*
 * find - finds a free block that can satisfy the umalloc request.
 * design decision -    if an appropriate block cannot be found, return the final block
 *                      the final block will not be returned under any other circumstance
 */
memory_block_t *find(size_t size) {
    memory_block_t *cmpr = free_head;
    while(cmpr->next && get_size(cmpr->next) < sizeof(memory_block_t) + size) { 
        cmpr = cmpr->next;
    }
    return cmpr;
}

/*
 * extend - extends the heap if more memory is required.
 * design decision - return an allocated block.
 */
memory_block_t *extend(size_t size) {
    const int max_call = 16 * PAGESIZE - sizeof(memory_block_t);
    const int shift_by = 4; // x16 adds a substantial amount of heap in comparison to the current space
    assert(size < max_call);
    int call = size << shift_by;
    while (call >= max_call) { // decrease the amount of heap being added if it exceeds the maximum
        call = call >> 1;
    }
    
    memory_block_t *ptr = csbrk(sizeof(memory_block_t) + call);
    put_block(ptr, call, true);
    if (size == call) {
        return ptr;
    }
    ufree(get_payload(ptr));
    return split(ptr, size);
}

/*
 * split - splits a given block in parts, one allocated, one free.
 */
memory_block_t *split(memory_block_t *block, size_t size) {
    assert(size < get_size(block)); 
    if (get_size(block) - size >= sizeof(memory_block_t *)) {
        block->block_size_alloc = (get_size(block) - (size + sizeof(memory_block_t *))) & 0x0FFFFFFF;
        block = (memory_block_t *) (((char *) get_payload(block)) + get_size(block));
    }
    put_block(block, size, true);
    return block;
}

/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */
int uinit() {
    free_head = csbrk(sizeof(memory_block_t));
    if (!free_head) {
        return -1;
    }
    put_block(free_head, 0, false);
    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size) {
    size = ALIGN(size);
    memory_block_t *cmpr = find(size);
    if (!cmpr->next) {
        cmpr = extend(size);
    } else if (get_size(cmpr->next) > sizeof(memory_block_t) + size) {
        cmpr = split(cmpr->next, size);
    } else {
        memory_block_t *temp = cmpr->next;
        cmpr->next = temp->next;
        cmpr = temp;
        allocate(cmpr);
    }
    return get_payload(cmpr);
}

/*
 * ufree -  frees the memory space pointed to by ptr, which must have been called
 * by a previous call to malloc.
 */
void ufree(void *ptr) {
    memory_block_t *ptrt = get_block(ptr);
    if (is_allocated(ptrt)) {
        deallocate(ptrt);
        memory_block_t *cmpr = free_head;
        if (!cmpr->next) {
            cmpr->next = ptrt;
            return;
        } 
        while (cmpr->next && cmpr->next < ptrt) {
                cmpr = cmpr->next;
        }

        // test prev -> pointer coalescment
        if (((char *)ptrt) - ((char *)get_payload(cmpr)) == get_size(cmpr)) {
            cmpr->block_size_alloc = (get_size(cmpr) + sizeof(memory_block_t) + get_size(ptrt)) & 0x0FFFFFFF;
        } else {
            // add the pointer to the free list
            if (cmpr->next) {
                memory_block_t *temp = cmpr->next;
                cmpr->next = ptrt;
                ptrt->next = temp;
            } else {
                cmpr->next = ptrt;
            }
            cmpr = cmpr->next;
        }
        // test pointer -> next coalescment
        if (cmpr->next && ((char *) cmpr->next) - ((char *) get_payload(cmpr)) == get_size(cmpr)) {
            cmpr->block_size_alloc = (get_size(cmpr) + sizeof(memory_block_t) + get_size(cmpr->next)) & 0x0FFFFFFF;
            memory_block_t *temp = cmpr->next;
            cmpr->next = temp->next;
        }
    }   
}