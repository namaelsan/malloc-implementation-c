#include "mymalloc.h"

#include <stdio.h>

/** finds a block based on strategy,
 * if necessary it splits the block,
 * allocates memory,
 * returns the start addrees of data[]*/
void *mymalloc(size_t size) { 
    return NULL; 
}

/** frees block,
 * if necessary it coalesces with negihbors,
 * adjusts the free list
 */
void myfree(void *p) {

}

/** splits block, by using the size(in 16 byte blocks)
 * returns the left block,
 * make necessary adjustments to the free list
 */
Block *split_block(Block *b, size_t size) { 
    return NULL; 
}

/** coalesce b with its left neighbor
 * returns the final block
 */
Block *left_coalesce(Block *b) { 
    return NULL; 
}

/** coalesce b with its left neighbor
 * returns the final block
 */
Block *right_coalesce(Block *b) { 
    return NULL; 
}

/** for a given block returns its next block in the list*/
Block *next_block_in_freelist(Block *b) { 
    return NULL; 
}

/** for a given block returns its prev block in the list*/
Block *prev_block_in_freelist(Block *b) { 
    return NULL; 
}

/** for a given block returns its right neghbor in the address*/
Block *next_block_in_addr(Block *b) { 
    return NULL; 
}

/** for a given block returns its left neghbor in the address*/
Block *prev_block_in_addr(Block *b) { 
    return NULL; 
}

/**for a given size in bytes, returns number of 16 blocks*/
uint64_t numberof16blocks(size_t size_inbytes) { 
    return 0; 
}

/** prints meta data of the blocks
 * --------
 * size:
 * free:
 * --------
 */
void printheap() {

}

ListType getlisttype() { 
    return listtype; 
}

int setlisttype(ListType listtype) { 
    return 0; 
}

Strategy getstrategy() { 
    return strategy; 
}

int setstrategy(Strategy strategy) { 
    return 0; 
}