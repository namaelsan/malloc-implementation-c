/*
Enhar Apuhan 22120205012
Mervenur Saraç 22120205055

*/



#include "mymalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define true 1
#define false 0

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

Block *find_free_block(size_t block_count){
    Block *b=free_list;
    Block *min,*max;
    size_t size=block_count*16;

    if(getstrategy() == BEST_FIT){
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
        b = last_freed; // aramayı en son free edilmiş blocktan başlat

        while(b < heap_end && b!= NULL){// last_freed'den heap_end'e kadar kontrol et
            if(b->info.size >= size){ 
            return b;  // uygun değer bulunursa return et
            }
            b = next_block_in_freelist(b);   	
        }
        // uygun block bulunamadıysa
        b = heap_start; // b, heap_start değerine sıfırlanır
        while(b < last_freed && b!= NULL){  // başlangıçtan last_freed'e kadar kontrol et
            if(b->info.size >= size){ // uygun değer bulunursa return et
            return b;  
            }	    			
            b = next_block_in_freelist(b); 
        }
        return NULL; //tüm list iki döngü ile gezilmiş ama uygun block bulunamamışsa NULL return et
    }
    
    else if(getstrategy() == FIRST_FIT){
     
        while(b < heap_end && b!= NULL){
            if(b->info.size >= size){
                return b;
            }
            b = next_block_in_freelist(b);    
        }
        return NULL;
    }
        
    else if(getstrategy() == WORST_FIT){

        max = b; // max ilk bloktan başlar
        while(b < heap_end && b != NULL){
            if(b->info.size > max->info.size && b->info.size >= size){
                max = b;
            }
            b = next_block_in_freelist(b);     
        }
        if (max != NULL) { // uygun block bulunmuştur
            return max;
        } else {
            return NULL; // listede uygun block yoktur null dönderilir
        } 
    }
    return NULL;
}

/*  */
void add_free_list(Block *b){
    Block *temp = free_list;
    Block *next,*prev;
    b->info.isfree=true;

    if(free_list == NULL){
        free_list = b;
    }

    else if(getlisttype() == UNORDERED_LIST){
        while(temp != NULL && temp->next != NULL){
            temp = temp->next;
        }
        if(temp != b)
            temp->next = b;

    }else if(getlisttype() == ADDR_ORDERED_LIST){
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
/*kullanıcının istediiği boyutta bir block yapısı oluşturulur*/
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

    add_free_list(b);

    return b;
}
/**mymalloc fonksiyonu ilk defa çağrıdığında sbrk ile bellekten büyük bir alan ayrılır
*heap için başlangıç ve bitiş bilgileri atanır
*istenilen alan için blok oluşturulur
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
void myfree(void *p) {
    Block *b=(Block *)((char *)p - sizeof(Block)) ;
    Block *left,*right;
    b->info.isfree = true;


    left=left_coalesce(b);
    right=right_coalesce(b);
    add_free_list(b);

}



/** splits block, by using the size(in 16 byte blocks)
 * returns the left block,
 * make necessary adjustments to the free list
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
Block *left_coalesce(Block *b) {
    Block *left = prev_block_in_addr(b); // birleştirilecek olan block
    if (left != NULL && left->info.isfree == true) { // sol block varsa ve free ise birleştirilecek
            
        left->info.size = left->info.size + b->info.size + sizeof(Block) + sizeof(Tag); // sol bloğun yeni boyutu
            
        Tag *newtag = (Tag *)((char *)left + left->info.size + sizeof(Block)); // sol bloğun tag yapısı güncellenir
        newtag->size = left->info.size;
        newtag->isfree = true;

        free_block_from_list(b); // sağdaki blocku listeden çıkarabiliriz
        return left;
    }
    return NULL; // sol block uygun değilse NULL döndürülür
}

/** coalesce b with its right neighbor
 * returns the final block
 */
Block *right_coalesce(Block *b) {
    Block *right = next_block_in_addr(b); // birleştirilecek olan block
    if (right != NULL && right->info.isfree == true) { // sağ block varsa ve free ise
            
        b->info.size = b->info.size + right->info.size + sizeof(Block) + sizeof(Tag); // b'nin yeni boyutu sağ blockla total boyut olacak
            
        Tag *newtag = (Tag *)((char *)b + b->info.size + sizeof(Block)); // tag yapısı güncellenir
        newtag->size = b->info.size;
        newtag->isfree = true;

        free_block_from_list(right); // sağ bloğun artık free_listte olmasına gerek yok
        return b; // yeni block return edilir
    }
    return NULL; // sağ blok uygun değilse NULL döndürülür
}


/** for a given block returns its next block in the list*/
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
Block *next_block_in_addr(Block *b) { 
    Block *new=(Block *)((char *)b + sizeof(Block) + b->info.size +b->info.padding + sizeof(Tag));
    return new;
}

/** for a given block returns its left neghbor in the address*/
Block *prev_block_in_addr(Block *b) { 
    Tag *tag=(Tag *)((char *)b -sizeof(Tag));
    if(tag < heap_start || tag == NULL){
        return NULL;
    }
    Block *new=(Block *)((char *)tag -(tag->size + tag->padding + sizeof(Block)));
    return new;
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



int main(){
    
    setlisttype(UNORDERED_LIST);
    setstrategy(WORST_FIT);

    Block *b1,*b2,*b3,*b4;

    char *c1 = mymalloc(sizeof(char));
    b1=(char *)c1 - sizeof(Block);
    *c1='a';

    char *c2 = mymalloc(sizeof(char));
    b2=(char *)c2 - sizeof(Block);
    *c2='x';

    char *c3 = mymalloc(sizeof(char));
    b3=(char *)c3 - sizeof(Block);
    *c3='S';
    

    myfree(c1);
    c1=mymalloc(sizeof(char)*18);
    b4=(char *)c1 - sizeof(Block);

    c1[0]='a';
    c1[1]='b';
    c1[2]='c';
    c1[3]=0;

    
    printheap();

    return 0;
}
