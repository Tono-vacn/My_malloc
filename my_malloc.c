#include "my_malloc.h"
#include <stdlib.h>
#include <limits.h>
#include <time.h>

unsigned long given_mem = 0;
unsigned long free_mem = 0;


void sol_mem_blk(mem_blk * cur){
  if(cur==NULL){
    perror("invalid block to set sol");
    return;
  }
  cur->prev=NULL;
  cur->next=NULL;
}

void set_blk_size(mem_blk * cur, size_t size){
  if(size<=0||cur==NULL){
    perror("invalid block to set size");
    return;
  }
  cur->size = size;
}

void plus_given_mem(size_t size){
  given_mem+=size;
}

void plus_free_mem(size_t size){
  free_mem+=size;
}

void minus_free_mem(size_t size){
  free_mem-=size;
}

mem_blk * get_new_mem_blk(size_t size){
  if(size>0){
    mem_blk* new_mem_blk = (mem_blk*)sbrk(size+sizeof(mem_blk));

    if(new_mem_blk==(void*)-1){
      perror("unable to perform sbrk");
      return NULL;
    }
    set_blk_size(new_mem_blk,size);
    sol_mem_blk(new_mem_blk);
    //given_mem+=(size+sizeof(mem_blk));
    plus_given_mem(size+sizeof(mem_blk));
    return new_mem_blk;
  }
  else{
    perror("invalid size for new mem_blk");
    return NULL;
  }
}

mem_blk * divide_mem_blk(mem_blk * cur, size_t size){
if(cur==NULL || size<=0){
  perror("invalid mem_blk to divide");
  return NULL;
}else{
  //free_mem -=(size+sizeof(mem_blk));
  minus_free_mem(size+sizeof(mem_blk));
  set_blk_size(cur,cur->size-(size+sizeof(mem_blk)));
  mem_blk * new_mem_blk = (mem_blk *)((void*)cur+sizeof(mem_blk)+cur->size);
  sol_mem_blk(new_mem_blk);
  set_blk_size(new_mem_blk, size);
  return new_mem_blk;
}
}

mem_blk * merge_mem_blk(mem_blk* prv, mem_blk * nxt){
  if(prv==NULL){
    return nxt;
  }else if (nxt==NULL){
    return prv;
  }else if ((void*)prv+prv->size+sizeof(mem_blk)==(void*)nxt){
    set_blk_size(prv,prv->size+nxt->size+sizeof(mem_blk));
    if(nxt->next!=NULL){
      nxt->next->prev = prv;
      prv->next = nxt->next;
    }else{
      prv->next = NULL;
    }
    sol_mem_blk(nxt);
    return prv;
  }else{
    return NULL;
  }
  
}

void use_mem_blk(mem_blk * cur){
  if(cur==NULL){
    perror("invalid mem_blk to use from free mem_blk list");
    //return NULL;
  }else{
  if(cur->prev==NULL){
    head_origin = cur->next;
  }else{
    cur->prev->next = cur->next;
  }

  if(cur->next!=NULL){
    cur->next->prev = cur->prev;
  }
  sol_mem_blk(cur);
  minus_free_mem(cur->size+sizeof(mem_blk));
  //free_mem-=(cur->size+sizeof(mem_blk));
  }

}

void back_to_free(mem_blk * cur){
  if(cur==NULL){
    perror("invalid mem_blk to free");
  }

  if(head_origin==NULL){
    sol_mem_blk(cur);
    head_origin = cur;
    plus_free_mem(cur->size+sizeof(mem_blk));
    //free_mem +=(cur->size+sizeof(mem_blk));
  }else{
    mem_blk * ptr = head_origin;
    mem_blk * ptr_prv = NULL;
    while(ptr!=NULL){
      if(cur<ptr){
        break;
      }
      ptr_prv = ptr;
      ptr = ptr->next;
    }
    cur->next = ptr;
    cur->prev = ptr_prv;
    if(ptr){
      ptr->prev = cur;
    }

    if(ptr_prv){    
        ptr_prv->next = cur;
      }
      else{
        head_origin = cur;
      }

    //free_mem +=(cur->size+sizeof(mem_blk));
    plus_free_mem(cur->size+sizeof(mem_blk));

    //merge
    if(ptr){
      merge_mem_blk(cur,ptr);
    }

    if(ptr_prv){
      merge_mem_blk(ptr_prv,cur);
    }

  }


}

void * divide_or_not(mem_blk * cur, size_t size){
    if(cur->size>size+sizeof(mem_blk)){
      mem_blk * new_mem_blk = divide_mem_blk(cur, size);
      return (void*) new_mem_blk+sizeof(mem_blk);
    }else{
      use_mem_blk(cur);
      return (void*)cur+sizeof(mem_blk);
    }
}

mem_blk * get_first_fit(size_t size){
  if(size<=0){
    perror("invalid input size for ff_malloc");
    return NULL;
  }
  mem_blk * cur = head_origin;
  while(cur!=NULL){
    if(cur->size>=size){
      return cur;
    }
    cur = cur->next;
  }
  return NULL;
}

void * ff_malloc(size_t size){
  if(size<0){
    perror("invalid input size");
    return NULL;
  }

  mem_blk * first_fit = get_first_fit(size);
  if(first_fit==NULL){
    mem_blk * res = get_new_mem_blk(size);
    return (void*)res + sizeof(mem_blk);
  }else{
    return divide_or_not(first_fit, size);
  }

  // mem_blk * cur = head_origin;
  // while(cur!=NULL){
  //   if(cur->size>=size){
  //     return divide_or_not(cur, size);
  //   }
  //   cur = cur->next;
  // }

  //mem_blk * res = get_new_mem_blk(size);
  //return (void*)res + sizeof(mem_blk);
}


void simple_free(void * ptr){
  if(ptr==NULL){
    perror("invalid value to free");
    return;
  }
  mem_blk * to_be_freed = (mem_blk*)(ptr-sizeof(mem_blk));
  back_to_free(to_be_freed);
}

void ff_free(void * ptr){
  simple_free(ptr);
}

mem_blk * get_best_fit(size_t size){
  if(size<=0){
    perror("invalid input size for bf_malloc");
    return NULL;
  }
  mem_blk * cur = head_origin;
  mem_blk * rec = NULL;
  size_t rec_size = INT_MAX;
  while(cur!=NULL){
    if(cur->size>=size){
      if(cur->size==size){
        return cur;
      }
      else if(cur->size>size && cur->size<rec_size){
        rec_size = cur->size;
        rec = cur;
      }
    }
    cur= cur->next;
  }
  return rec;

}

void * bf_malloc(size_t size){
  if(size<0){
    perror("invalid input size for bf_malloc");
    return NULL;
  }
  mem_blk* best_fit = get_best_fit(size);
  if (best_fit==NULL){
    mem_blk * res = get_new_mem_blk(size);
    return (void*)res+sizeof(mem_blk);
  }else{
    return divide_or_not(best_fit,size);
  }
}

void bf_free(void * ptr){
  simple_free(ptr);
}

unsigned long get_data_segment_free_space_size(){
  return free_mem;
}

unsigned long get_data_segment_size(){
  return given_mem;
}




// /////test code for debug////
// #define NUM_ITERS    100
// #define NUM_ITEMS    10000

// //#ifdef FF
// #define MALLOC(sz) ff_malloc(sz)
// #define FREE(p)    ff_free(p)
// // #endif
// // #ifdef BF
// // #define MALLOC(sz) bf_malloc(sz)
// // #define FREE(p)    bf_free(p)
// // #endif


// double calc_time(struct timespec start, struct timespec end) {
//   double start_sec = (double)start.tv_sec*1000000000.0 + (double)start.tv_nsec;
//   double end_sec = (double)end.tv_sec*1000000000.0 + (double)end.tv_nsec;

//   if (end_sec < start_sec) {
//     return 0;
//   } else {
//     return end_sec - start_sec;
//   }
// };


// struct malloc_list {
//   size_t bytes;
//   int *address;
// };
// typedef struct malloc_list malloc_list_t;

// malloc_list_t malloc_items[2][NUM_ITEMS];

// unsigned free_list[NUM_ITEMS];


// int main(int argc, char *argv[])
// {
//   int i, j, k;
//   unsigned tmp;
//   unsigned long data_segment_size;
//   unsigned long data_segment_free_space;
//   struct timespec start_time, end_time;

//   srand(0);

//   const unsigned chunk_size = 32;
//   const unsigned min_chunks = 4;
//   const unsigned max_chunks = 16;
//   for (i=0; i < NUM_ITEMS; i++) {
//     malloc_items[0][i].bytes = ((rand() % (max_chunks - min_chunks + 1)) + min_chunks) * chunk_size;
//     malloc_items[1][i].bytes = ((rand() % (max_chunks - min_chunks + 1)) + min_chunks) * chunk_size;
//     free_list[i] = i;
//   } //for i

//   i = NUM_ITEMS;
//   while (i > 1) {
//     i--;
//     j = rand() % i;
//     tmp = free_list[i];
//     free_list[i] = free_list[j];
//     free_list[j] = tmp;
//   } //while

// printlist();

//   for (i=0; i < NUM_ITEMS; i++) {
//     malloc_items[0][i].address = (int *)MALLOC(malloc_items[0][i].bytes);
//   } //for i

// printlist();

//   //Start Time
//   //clock_gettime(CLOCK_MONOTONIC, &start_time);

//   for (i=0; i < NUM_ITERS; i++) {
//     unsigned malloc_set = i % 2;
//     for (j=0; j < NUM_ITEMS; j+=50) {
//       for (k=0; k < 50; k++) {
// 	unsigned item_to_free = free_list[j+k];
// 	FREE(malloc_items[malloc_set][item_to_free].address);
//       } //for k

//       printlist();
//       for (k=0; k < 50; k++) {
// 	malloc_items[1-malloc_set][j+k].address = (int *)MALLOC(malloc_items[1-malloc_set][j+k].bytes);
//       } //for k
//     } //for j
//   } //for i


// printlist();
//   //Stop Time
//   //clock_gettime(CLOCK_MONOTONIC, &end_time);

//   data_segment_size = get_data_segment_size();
//   data_segment_free_space = get_data_segment_free_space_size();
//   printf("data_segment_size = %lu, data_segment_free_space = %lu\n", data_segment_size, data_segment_free_space);

//   double elapsed_ns = calc_time(start_time, end_time);
//   printf("Execution Time = %f seconds\n", elapsed_ns / 1e9);
//   printf("Fragmentation  = %f\n", (float)data_segment_free_space/(float)data_segment_size);

//   mem_blk * ptr = head_origin;
//   unsigned long res = 0;
//   while(ptr!=NULL){
//     res+=(ptr->size+sizeof(*ptr));
//     ptr=ptr->next;
//   }

//   for (i=0; i < NUM_ITEMS; i++) {
//     FREE(malloc_items[0][i].address);
//   } //for i

//   return 0;
// }
