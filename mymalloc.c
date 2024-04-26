#include "mymalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define true 1
#define false 0

void free_block_from_list(Block *b){
        if(b==free_list)
            free_list=next_block_in_freelist(b);
        Block *prev=b->prev;
        Block *next=b->next;
        prev->next=next;
        next->prev=prev;
        b->prev=NULL;
        b->next=NULL;
}

Block *find_free_block(size_t block_count){
    Block *b=free_list;
    Block *prev,*next,*new,*temp,*min;
    size_t size=block_count*16;

    if(getstrategy()==BEST_FIT){
        min=b;

        while(b<heap_end && b!= NULL){
            if((b->info.size < min->info.size) &&(min->info.size >= size)){
                min=b;
            }
            b=next_block_in_freelist(b);
        }
        new=min;


        return NULL;
    }
    else if(getstrategy()== NEXT_FIT){

    }
    else if(getstrategy()== FIRST_FIT){

    }
    else if(getstrategy()== WORST_FIT){

    }
}

Block *create_block(Block *b,size_t data_size){
    size_t size=data_size-(sizeof(Block)+sizeof(Tag));
    b->next=next_block_in_freelist(b);
    b->prev=prev_block_in_freelist(b);
    b->info.isfree=true;
    b->info.size=size;
    b->info.padding=0;

    Tag* newtag=(char *)b + size + sizeof(Block);
    newtag->isfree=true;
    newtag->size=size;
    newtag->padding=0;

    Block *prev=free_list;
    while(next_block_in_freelist(prev)){
        prev=next_block_in_freelist(prev);
    }
    prev->next=b;

    return b;
}

Block *expandheap(size_t size){
        char *start = sbrk(HEAP_SIZE);
        if (start == -1) {
            perror("sbrk error: not available memory");
            return NULL;
        }
        heap_end = start + size;
        Block *b = create_block(start, size);
        return b;
}

/** finds a block based on strategy,
 * if necessary it splits the block,
 * allocates memory,
 * returns the start addrees of data[]*/
void *mymalloc(size_t size) { 
    static int first=true;
    Block *b;
    if (first){
        heap_start=expandheap(HEAP_SIZE);
        first=false;
        heap_end=(char *)heap_start+HEAP_SIZE;
        free_list=heap_start;
    }

    int block_count=numberof16blocks(size);
    b = find_free_block(block_count);

    if(b->info.size >= size){
        if(b->info.size >= size + sizeof(Block) + sizeof(Tag)){
            b=split_block(b,block_count);
        }
        else{
            free_block_from_list(b);
        }
        return b;
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
void myfree(void *p) {

}

/** splits block, by using the size(in 16 byte blocks)
 * returns the left block,
 * make necessary adjustments to the free list
 */
Block *split_block(Block *b, size_t block_count) { 
    size_t size=block_count*16;

    if (b->info.size < (char *)size +sizeof(Block) +sizeof(Tag))
        return NULL; 

    int newbsize=b->info.size -size -sizeof(Block) -sizeof(Tag);

    Block *new=(char *)b + sizeof(Block) + newbsize +sizeof(Tag);
    Tag *newtag=((char *)new-sizeof(Tag));

    b->info.size=newbsize;
    newtag->size=newbsize;
    newtag->isfree=b->info.isfree;
    newtag->padding=0;

    new->info.isfree=false;
    new->prev=NULL;
    new->next=NULL;
    new->info.size=size;
    new->info.padding=0;

    Tag *oldtag=(char *)new + sizeof(Block) + sizeof(Tag) + new->info.size;
    oldtag->isfree=new->info.isfree;
    oldtag->padding=new->info.padding;
    oldtag->size=new->info.size;
    return new;
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
    Block *temp=b;
    if(b->next==NULL){
        while(temp < heap_end && temp != NULL){
            temp=next_block_in_addr(temp);
            if(temp->info.isfree){
                return temp;
            }
        }
        return NULL;
    }
    return b->next; 
}

/** for a given block returns its prev block in the list*/
Block *prev_block_in_freelist(Block *b) { 
    Block *temp=b;
    if(b->prev==NULL){
        while(temp >= heap_start && temp != NULL){
            temp=prev_block_in_addr(temp);
            if(temp->info.isfree){
                return temp;
            }
        }
        return NULL;
    }
    return b->prev; 
}

/** for a given block returns its right neghbor in the address*/
Block *next_block_in_addr(Block *b) { 
    Block * new=b + sizeof(Block) + b->info.size +b->info.padding;
    return b;
}

/** for a given block returns its left neghbor in the address*/
Block *prev_block_in_addr(Block *b) { 
    Block * new=b + sizeof(Block) + b->info.size +b->info.padding;
    return b;
}

/**for a given size in bytes, returns number of 16 blocks*/
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
void printheap() {
    Block *current=heap_start;
    printf("Blocks\n\n");
    while(current!=heap_end){
        printf("Free: %d\n\nSize: %d\n\n---------------\n\n",current->info.isfree,current->info.size);
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



void main(){

    char *test=mymalloc(sizeof(char));
    *test=1;
    char *ch=mymalloc(sizeof(char));
    *ch=2;
    char *ch2=mymalloc(sizeof(char));
    *ch2=2;
    printf("%d,%p",*test,test);
    printf("%d,%p",*ch,ch);
    printf("%d,%p",*ch2,ch2);

    myfree(ch);
    ch=mymalloc(sizeof(char));
    *ch=2;  
    printf("%d,%p",*ch,ch);

    exit(0);
}