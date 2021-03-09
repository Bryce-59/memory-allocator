
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
    bool consistent = false;
    // 1: ensure that the memory list remains in memory order
    memory_block_t *test_one = first_head;
    while (test_one->next) {
        consistent = consistent || (test_one - test_one->next <= 0);
        test_one = test_one->next;
    }
    // 2: ensure that the alignment is correct for all blocks
    memory_block_t *test_two = first_head;
    while (test_two->next) {
        consistent = consistent || (((unsigned long) test_two & 0xFF) == (unsigned long) test_two);
        test_two = test_two->next;
    }
    // 3: ensure that there is no overlap between blocks of memory
    memory_block_t *test_three = first_head;
    while (test_three->next) {
        int end_bit = (unsigned long) test_three + get_size(test_three);
        consistent = consistent || (unsigned long) test_three->next > end_bit;
        test_three = test_three->next;
    }
    // 4: ensure that there are no free blocks that are adjacent in memory (coalescence check)
    memory_block_t *test_four = first_head;
    while (test_four->next) {
        if (!is_allocated(test_four) && !is_allocated(test_four->next)) {
            consistent = consistent || test_four + get_size(test_four) != test_four->next;
        }
        test_four = test_four->next;
    }
    return !consistent;
}