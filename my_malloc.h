#ifndef __MYALLOC_H__
#define __MYALLOC_H__

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

//First fit
void * ff_malloc(size_t size);
void ff_free(void * ptr);

//Best fit

void * bf_malloc(size_t size);
void bf_free(void * ptr);

unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();

//struct for memory
struct _mem_blk
{
  size_t size;
  struct _mem_blk * next;
  struct _mem_blk * prev;
};

typedef struct _mem_blk mem_blk;

// list head
mem_blk * head_origin = NULL;

//helper functions
void sol_mem_blk(mem_blk * cur);
void set_blk_size(mem_blk * cur, size_t size);
void * divide_or_not(mem_blk * cur, size_t size);
void plus_given_mem(size_t size);
void plus_free_mem(size_t size);
void minus_free_mem(size_t size);
mem_blk * get_new_mem_blk(size_t size);
mem_blk * divide_mem_blk(mem_blk * cur, size_t size);
mem_blk * merge_mem_blk(mem_blk * prv, mem_blk* nxt);
void use_mem_blk(mem_blk* cur);
void back_to_free(mem_blk * cur);
mem_blk * get_best_fit(size_t size);
mem_blk * get_first_fit(size_t size);
#endif