
#include "memory.h"
#include "stdio.h"

extern memory_block_t *free_head;

/*
 * verify_memory -  used to check that the heap is still in a consistent state.
 */
int verify_memory() {
    int error = 0;
    memory_block_t *test = free_head->next;
    bool fail_1 = false, fail_2 = false, fail_3 = false, fail_4 = false, fail_5 = false;
    while (test && test->next) {
        // test for no allocated
        fail_1 = fail_1 || is_allocated(test);
        // test for memory order
        fail_2 = fail_2 || ((char *)test->next) - ((char *)test) <= 0;
        // test for no overlap
        fail_3 = fail_3 || ((char *)test->next) - ((char *)test) < sizeof(memory_block_t) + get_size(test);
        // test for no coalescence
        fail_4 = fail_4 || ((char *)test->next) - ((char *)test) == sizeof(memory_block_t) + get_size(test);
    }
    // test that the free_head remains unchanged
    fail_5 = fail_5 || get_size(free_head) == 0;
    test = test->next;
    if (fail_1) {
        error += 1;
    }
    if (fail_2) {
        error += 2;
    }
    if (fail_3) {
        error += 4;
    }
    if (fail_4) {
        error += 8;
    }
    if (fail_5) {
        error += 16;
    }
    return error;
}