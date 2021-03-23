
#include "umalloc.h"
#include "stdio.h" //

//Place any variables needed here from umalloc.c as an extern.
extern memory_block_t *free_head;

/*
 * check_heap -  used to check that the heap is still in a consistent state.
 * Required to be completed for checkpoint 1.
 * Should return 0 if the heap is still consistent, otherwise return a non-zero
 * return code. Asserts are also a useful tool here.
 */
int check_heap() {
    int error = 0;
    memory_block_t *test = free_head;
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



/*    // the following tests might be coalesced later, but will
    // remain like this for now for the sake of readability--
    bool inconsistent = false;
    // 1: ensure that the memory list remains in memory order
    memory_block_t *test_one = first_head;
    while (test_one->next) {
        inconsistent = inconsistent || (test_one - test_one->next >= 0); // TRUE if fail
        test_one = test_one->next;
    }
    // 2: ensure that there is no overlap between blocks of memory
    memory_block_t *test_two = first_head;
    while (test_two->next) {
        int end_bit = (unsigned long) test_two + get_size(test_two);
        inconsistent = inconsistent || (unsigned long) test_two->next <= end_bit; // TRUE if fail
        test_two = test_two->next;
    }
    // 3: ensure that the alignment is correct for all blocks (should align to 16 or 0xFF)
    memory_block_t *test_three = first_head;
    while (test_three->next) {
        inconsistent = inconsistent || (((unsigned long) test_three & ALIGNMENT) != (unsigned long) test_three); // TRUE if fail
        test_three = test_three->next;
    }
    // 4: ensure that there are no free blocks that are adjacent in memory (coalescence check)
    memory_block_t *test_four = first_head;
    while (test_four->next) {
        if (!is_allocated(test_four) && !is_allocated(test_four->next)) {
            inconsistent = inconsistent || test_four + get_size(test_four) == test_four->next; // TRUE if fail
        }
        test_four = test_four->next;
    }
    return !inconsistent; //negate "inconsistent" to get "consistent"*/
    return 1;
}