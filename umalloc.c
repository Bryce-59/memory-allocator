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
int put_block(memory_block_t *block, size_t size, bool alloc) {
    if (block == NULL || size % ALIGNMENT != 0 || alloc >> 1 != 0) {
        return -1;
    }
    block->block_size_alloc = size | alloc;
    block->next = NULL;
    return 0;
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
 */
memory_block_t *find(size_t size) {
    return NULL;
}

/*
 * extend - extends the heap if more memory is required.
 * design decision - return an allocated block.
 */
memory_block_t *extend(size_t size) {
    size = ALIGN(size);
    memory_block_t *ptr = csbrk(sizeof(memory_block_t) + size);
    assert(put_block(ptr, size, true) == 0);
    return ptr;
}

/*
 * split - splits a given block in parts, one allocated, one free.
 */
memory_block_t *split(memory_block_t *block, size_t size) {
    size = ALIGN(size);
    if (get_size(block) - size >= sizeof(memory_block_t *)) {
        block->block_size_alloc = (get_size(block) - (size + sizeof(memory_block_t *))) & 0x0FFFFFFF;
        block = (memory_block_t *) (((char *) get_payload(block)) + get_size(block));
    }
    assert(put_block(block, size, true) == 0);
    return block;
}

/*
 * coalesce - coalesces a free memory block with neighbors.
 */
memory_block_t *coalesce(memory_block_t *block) {
    return NULL;
}

/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */
int uinit() {
    free_head = csbrk(sizeof(memory_block_t));
    return put_block(free_head,0,false);
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size) {
    size = ALIGN(size);
    memory_block_t *cmpr = free_head;
    if (cmpr) {
        while(cmpr->next && get_size(cmpr->next) < sizeof(memory_block_t) + size) { 
            cmpr = cmpr->next;
        }
    }
    
    if (!cmpr || !cmpr->next) {
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
 * NOTE: Add coalesce
 */
void ufree(void *ptr) {
    memory_block_t *ptrt = get_block(ptr);
    deallocate(ptrt);
    memory_block_t *cmpr = free_head;
    if (cmpr) {
        while (cmpr->next && cmpr->next < ptrt) {
            cmpr = cmpr->next;
        }
    } else {
        free_head = ptrt;
    }
    
    if (ptrt != cmpr->next) {
        // test prev -> pointer coalescment
        if (((memory_block_t *) ((char *) get_payload(cmpr)) + get_size(cmpr)) == ptrt) {
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
        if (((memory_block_t *) ((char *) get_payload(cmpr)) + get_size(cmpr)) == cmpr->next) {
            cmpr->block_size_alloc = (get_size(cmpr) + sizeof(memory_block_t) + get_size(cmpr->next)) & 0x0FFFFFFF;
            memory_block_t *temp = cmpr->next;
            cmpr->next = temp->next;
        }
    }
}