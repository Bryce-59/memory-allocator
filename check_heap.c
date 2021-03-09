
#include "umalloc.h"

//Place any variables needed here from umalloc.c as an extern.
extern memory_block_t *first_head;

/*
 * check_heap -  used to check that the heap is still in a consistent state.
 * Required to be completed for checkpoint 1.
 * Should return 0 if the heap is still consistent, otherwise return a non-zero
 * return code. Asserts are also a useful tool here.
 */
int check_heap() {
    // the following tests might be coalesced later, but will
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
    return !inconsistent; //negate "inconsistent" to get "consistent"
}