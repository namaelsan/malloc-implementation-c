/*

28/04/2024

Enhar Apuhan 22120205012
Mervenur Saraç 22120205055

This program includes basic implementations of the mymalloc and myfree functions.
mymalloc function simply allocates memory of the requested size from the heap 
and myfree function deallocates the previously allocated memory, making it available again for use.

*/



#include "mymalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define true 1
#define false 0

/* removes the given block from free_list according to the listtype */
void free_block_from_list(Block *b){
        Block *prev;
        Block *next;

        if(b==free_list)
            free_list=next_block_in_freelist(b);

        if (getlisttype() == UNORDERED_LIST){    
            next=b->next;
            prev = b->prev;
        }
        else if(getlisttype() == ADDR_ORDERED_LIST){
        /* if listtype is ordered, finds the right location */
            next=next_block_in_freelist(b);
            prev = prev_block_in_freelist(b);
        }

        if(next != NULL)
            next->prev = prev;
        if(prev != NULL)
            prev->next=next;
        if(next == NULL && prev == NULL && b->info.isfree)
            free_list=NULL;

        b->info.isfree=false;
        b->prev=NULL;
        b->next=NULL;
 
}

/* finds the free block according to the strategy */
Block *find_free_block(size_t block_count){
    Block *b=free_list;
    Block *min,*max;
    size_t size=block_count*16;

    if(getstrategy() == BEST_FIT){
    /* best_fit returns the smallest free block in the free_list thet can provide the size */    
        min=b;

        while(b<heap_end && b!= NULL){
            if((min->info.size > b->info.size) && (b->info.size >= size)){
                min=b;
            }
            b=next_block_in_freelist(b);
        }
        if (min->info.size < size)
            return NULL;

        return min;
    }
    
    else if(getstrategy() == NEXT_FIT){
    /* next_fit returns the firts free block after the last freed block */
        b = last_freed; 

        while(b < heap_end && b!= NULL){
            if(b->info.size >= size){ 
            return b; 
            }
            b = next_block_in_freelist(b);   	
        }

        b = heap_start; 
        /* if it cannot find a block it checks all block from beginning of free_list to the last_freed */
        while(b < last_freed && b!= NULL){ 
            if(b->info.size >= size){ 
            return b;  
            }	    			
            b = next_block_in_freelist(b); 
        }
        return NULL; 
    }
    
    else if(getstrategy() == FIRST_FIT){
        /* first_fit returns the first block in the free_list that can provide the size */
     
        while(b < heap_end && b!= NULL){
            if(b->info.size >= size){
                return b;
            }
            b = next_block_in_freelist(b);    
        }
        return NULL;
    }
        
    else if(getstrategy() == WORST_FIT){
    /* worst_fit returns the biggest block in the free_list if it  can provide the size */    

        max = b; 
        while(b < heap_end && b != NULL){
            if(b->info.size > max->info.size && b->info.size >= size){
                max = b;
            }
            b = next_block_in_freelist(b);     
        }
        if (max != NULL) { 
            return max;
        } else {
            return NULL; 
        } 
    }
    return NULL;
}

/* freed block is added to free_list according to the listtype */
void add_free_list(Block *b){
    Block *temp = free_list;
    Block *next,*prev;
    b->info.isfree=true;

    if(free_list == NULL){
        free_list = b;
    }

    else if(getlisttype() == UNORDERED_LIST){ 
    /* if littype is unordered, block is added to end oof the list */    
        while(temp != NULL && temp->next != NULL){
            temp = temp->next;
        }
        if(temp != b)
            temp->next = b;

    }else if(getlisttype() == ADDR_ORDERED_LIST){
    /*if listtype is ordered, block is added to the list 
    * based on its address*/    
        next = next_block_in_freelist(b);
        if(next != NULL)
            next->prev = b;
        prev = prev_block_in_freelist(b);
        if(prev != NULL)
            prev->next = b;
        else
            free_list = b;

        b->prev = prev;
        b->next = next;
        last_freed = b;

    }
}
/* this function creates a user-specified block from large sbrk-allocated memory block
*/
Block *create_block(Block *b,size_t data_size){ 
    size_t size=data_size-(sizeof(Block)+sizeof(Tag));
    b->info.size=size;
    b->info.isfree=false;
    b->info.padding=0;
    b->next=next_block_in_freelist(b);
    b->prev=prev_block_in_freelist(b);


    Tag* newtag=(Tag *)((char *)b + size + sizeof(Block));
    newtag->isfree=false;
    newtag->size=size;
    newtag->padding=0;

    add_free_list(b); // the new block is added to free_list

    return b;
}
/** this function allocates a large block of memory from heap using sbrk,
 * and creates a block for it.
*/
Block *expandheap(size_t size){ 
        Block *start = sbrk(HEAP_SIZE);
        if(!heap_start)
            heap_start = start;
        if ((uint64_t)start == -1) {
            perror("sbrk error: not available memory");
            return NULL;
        }
        heap_end = (Block *)((char *)start + size);
        start->info.isfree=false;
        Block *b = create_block(start, size);
        return b;
}

/** finds a block based on strategy,
 * if necessary it splits the block,
 * allocates memory,
 * returns the start addrees of data[]*/

/* initially allocates a large block from memory,
* then upon each call,organizes the most suitable free block
* and returns it
*/
void *mymalloc(size_t size) { 
    static int first=true;
    Block *b,*new;
    if (first){
        heap_start=expandheap(HEAP_SIZE);
        last_freed=heap_start;
        first=false;
        heap_end=(Block *)((char *)heap_start+HEAP_SIZE);
    }

    int block_count=numberof16blocks(size);
    b = find_free_block(block_count);

    if(b != NULL && b->info.size >= size){
        if(b->info.size >= size + sizeof(Block) + sizeof(Tag)){
            new=split_block(b,block_count);
            add_free_list(b);
            b=new;
        }
        else{
            free_block_from_list(b);
        }
        return (char *)b +sizeof(Block);
    }

    b=expandheap(HEAP_SIZE);
    if(b){
        return(mymalloc(size));
    }
    return NULL;

}

/** frees block,
 * if necessary it coalesces with negihbors,
 * adjusts the free list
 */

/* frees the given block,
* if block has a free neighbor it coalesces with it
* and block is added to free_list
*/
void myfree(void *p) {
    Block *b=(Block *)((char *)p - sizeof(Block));
    b->info.isfree = true;

    left_coalesce(b);
    right_coalesce(b);
    add_free_list(b);
}



/** splits block, by using the size(in 16 byte blocks)
 * returns the left block,
 * make necessary adjustments to the free list
 */

/* splits blocks for better memory useage,
* organizes new block information,
* and returns the block to be used.
*/
Block *split_block(Block *b, size_t block_count) { 
    size_t size=block_count*16;

    if (b->info.size < size +sizeof(Block) +sizeof(Tag))
        return NULL; 

    int newbsize=b->info.size -size -sizeof(Block) -sizeof(Tag);

    Block *new=(Block *)((char *)b + sizeof(Block) + newbsize +sizeof(Tag));
    Tag *newtag=(Tag *)((char *)new-sizeof(Tag));

    b->info.size=newbsize;
    newtag->size=newbsize;
    newtag->isfree=b->info.isfree;
    newtag->padding=0;

    new->info.isfree=false;
    new->prev=NULL;
    new->next=NULL;
    new->info.size=size;
    new->info.padding=0;

    Tag *oldtag=(Tag *)((char *)new + sizeof(Block) + new->info.size);
    oldtag->isfree=new->info.isfree;
    oldtag->padding=new->info.padding;
    oldtag->size=new->info.size;
    return new;
}

/** coalesce b with its left neighbor
 * returns the final block
 */

/* if there are two neighboring free blocks,
* this function coalesces the right block into the left block
 */
Block *left_coalesce(Block *b) {
    Block *left = prev_block_in_addr(b);
    if (left != NULL && left->info.isfree == true) {
            
        left->info.size = left->info.size + b->info.size + sizeof(Block) + sizeof(Tag);
            
        Tag *newtag = (Tag *)((char *)left + left->info.size + sizeof(Block));
        newtag->size = left->info.size;
        newtag->isfree = true;

        free_block_from_list(b);
        return left;
    }
    return NULL;
}

/** coalesce b with its right neighbor
 * returns the final block
 */

/* if there are two neighboring free blocks,
* this function coalesces the left block into the right block
*/
Block *right_coalesce(Block *b) {
    Block *right = next_block_in_addr(b); 
    if (right != NULL && right->info.isfree == true) { 
            
        b->info.size = b->info.size + right->info.size + sizeof(Block) + sizeof(Tag); 
            
        Tag *newtag = (Tag *)((char *)b + b->info.size + sizeof(Block)); 
        newtag->size = b->info.size;
        newtag->isfree = true;

        free_block_from_list(right); 

        return b; 
    }
    return NULL; 
}


/** for a given block returns its next block in the list*/

/* finds the next free block in heap and returns it */
Block *next_block_in_freelist(Block *b) {  
    Block *temp=b;
    if(b->next==NULL){
        while(temp < heap_end && temp != NULL){
            temp=next_block_in_addr(temp);
            if(temp->info.isfree && temp != heap_end){
                return temp;
            }
        }
        return NULL;
    }
    return b->next; 
}

/** for a given block returns its prev block in the list*/

/* finds the previos free block in heap and returns it */
Block *prev_block_in_freelist(Block *b) { 
    Block *temp=b;
    if(b->prev==NULL){
        while(temp >= heap_start && temp != NULL){
            temp=prev_block_in_addr(temp);
            if (temp == NULL){
                return NULL;
            }
            else if(temp->info.isfree){
                return temp;
            }
        }
        return NULL;
    }
    return b->prev; 
}

/** for a given block returns its right neghbor in the address*/

/* finds the next block in the heap and returns it */
Block *next_block_in_addr(Block *b) { 
    Block *new=(Block *)((char *)b + sizeof(Block) + b->info.size +b->info.padding + sizeof(Tag));
    return new;
}

/** for a given block returns its left neghbor in the address*/

/* finds the previous block in the heap and returns it */
Block *prev_block_in_addr(Block *b) { 
    Tag *tag=(Tag *)((char *)b -sizeof(Tag));
    if((char *)tag < (char *)heap_start || tag == NULL){
        return NULL;
    }
    Block *new=(Block *)((char *)tag -(tag->size + tag->padding + sizeof(Block)));
    return new;
}

/**for a given size in bytes, returns number of 16 blocks*/

/* calculates the number of 16 byte blocks required to the given byte size and returns it */
uint64_t numberof16blocks(size_t size_inbytes) { 
    size_inbytes+=16-(size_inbytes % 16);
    size_inbytes/=16;
    return size_inbytes;
}

/** prints meta data of the blocks
 * --------
 * size:
 * free:
 * --------
 */

/* prints meta data of all blocks in the heap */
void printheap() {
    Block *current=heap_start;
    printf("Blocks\n\n");
    while(current!=heap_end){
        printf("Free: %u\n\nSize: %lu\n\n---------------\n\n",current->info.isfree,current->info.size);
        current=next_block_in_addr(current);
    }
}

ListType getlisttype() { 
    return listtype; 
}

int setlisttype(ListType listtypenew) { 
    listtype = listtypenew;
    return 0; 
}

Strategy getstrategy() { 
    return strategy; 
}

int setstrategy(Strategy strategynew) { 
    strategy=strategynew;
    return 0; 
}


/* main function that calls mymalloc and myfree functions */
int main(){
    
    setlisttype(ADDR_ORDERED_LIST);
    setstrategy(NEXT_FIT);


    char *c1 = mymalloc(sizeof(char));
    *c1='a';

    char *c2 = mymalloc(sizeof(char));
    *c2='x';

    char *c3 = mymalloc(sizeof(char));
    *c3='S';
    

    myfree(c1);
    c1=mymalloc(sizeof(char)*18);

    c1[0]='a';
    c1[1]='b';
    c1[2]='c';
    c1[3]=0;

    
    printheap();

    return 0;
}
