#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

typedef struct memory
{
    size_t size;
    struct memory *prev;
    struct memory *next;
} memory_t;

size_t memory_size = sizeof(memory_t);
memory_t *first = NULL;
unsigned long free_mem = 0;
unsigned long total_memory = 0;
void *ff_malloc(size_t size);
void ff_free(void *ptr);
void *bf_malloc(size_t size);
void bf_free(void *ptr);
void *find_first_fit_in_freelist(size_t size);
void *find_best_fit_in_freelist(size_t size);
void *split_into_two(memory_t *block, size_t size);
void *remove_from_freelist(memory_t *block);
void *create_mem(size_t size);
void free_memory(void *ptr);
void insert_into_freelist(memory_t *prevOne, memory_t *nextOne, memory_t *ptr_begin);
void merge(memory_t *block);
unsigned long get_data_segement_size();
unsigned long get_data_segement_free_space_size();

void *ff_malloc(size_t size)
{
	// step1:
	memory_t *block = find_first_fit_in_freelist(size);

	// step2:
	if (block == NULL)
	{
		return create_mem(size);
	}

	if (block->size > size + memory_size)
	{
		return split_into_two(block, size);
	}
	else
	{
		return remove_from_freelist(block);
	}
}

// first fit strategy helper function:
// aiming to find the first position of the free space that meet the condition that block size >= request size
// general idea:
// declare a var point to the first pointer and start iteration
// in each iteration, we check if current block meet our condition, if so -> return block address
// otherwise -> we continue to scan free list
// after iteration, if we can not find any valid block, we return NULL
void *find_first_fit_in_freelist(size_t size)
{
	// corner case check
	if (size <= 0)
	{
		return NULL;
	}

	memory_t *curr = first;
	while (curr != NULL)
	{
		if (curr->size >= size)
		{
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

// first fit free function
// aiming to free given address memory
void ff_free(void *ptr)
{
	free_memory(ptr);
}

/************  main logic for best-fit strategy *************/

// main logic function for first_fit malloc
// general idea:
// step1: find the best_fit free space in free list by calling helper function "find_first_fit_in_freelist"
// step2: based on the block we get, process accordingly
//  case1: if required size < block's size < required size + memory_size ->
//      that means we can not have enough space to split it into 2 parts, but we can still malloc this space to use
//      we just call helper function "remove_from_freelist" to remove this free space
//  case2: if   block's size > require size + memory_size
//      that means we can split it into 2 parts, one to left for future malloc , another for current malloc
//      we will call helper function "split_into_two" to do that
//  case3: if  both case1 & 2 conditions not meet -> we can increase heap memory by calling helper function
//      "create_mem"
void *bf_malloc(size_t size)
{
	// step1:
	memory_t *block = find_best_fit_in_freelist(size);

	// step2:
	if (block == NULL)
	{
		return create_mem(size);
	}

	if (block->size > size + memory_size)
	{
		return split_into_two(block, size);
	}
	else
	{
		return remove_from_freelist(block);
	}
}

// best fit strategy helper function:
// aiming to find the first position of the free space that meet the condition that block size >= request size
// general idea:
// declare a var point to the first pointer and start iteration
// in each iteration, we check if current block meet our condition, if so -> return block address
// otherwise -> we continue to scan free list
// after iteration, if we can not find any valid block, we return NULL
void *find_best_fit_in_freelist(size_t size)
{
	// corner case check
	if (size <= 0)
	{
		return NULL;
	}

	memory_t *curr = first;
	memory_t *best = NULL;
	size_t largest_min = (size_t)-1;
	while (curr != NULL)
	{
		if (curr->size == size)
		{
			return curr;
		}
		else if (curr->size > size)
		{
			if (curr->size < largest_min)
			{
				largest_min = curr->size;
				best = curr;
			}
		}
		curr = curr->next;
	}

	if (best == NULL)
	{
		return NULL;
	}
	else
	{
		return best;
	}
}

// best fit free function
// aiming to free given address memory
void bf_free(void *ptr)
{
	free_memory(ptr);
}

/************  main logic for helper function *************/

// helper function:
// aiming to split current given block into 2 parts, 1 is for future malloc use anthoer is for current use
// general idea:

// Because current block size in this case now is: block-> size > size + memory_size
// we need to allocate (size + memory_size) for current use

// so the left size will be: block->size = block->size - size - memory_size
// And we want to return the memory address we allocate
// so we need to cal this position, I will call that pos is res
// so memory_t * res = (memory_t *)((void*) block + memory_size + block->size(which is after updated size))

// and we also need to set this struct field size as req "size"
// finally return allocation memory's address, i.e. (void*) res + memory_size

void *split_into_two(memory_t *block, size_t size)
{
	block->size = block->size - size - memory_size;
  free_mem -=(size+sizeof(memory_t));
	memory_t *res = (memory_t *)((void *)block + memory_size + block->size);
	res->size = size;
	return (void *)res + memory_size;
}

// helper function:
// aiming to allocate free memory space, in other words, remove block with the size from the freelist
// general idea:
// if removing block is first node, which means block->prev == NULL
// we should set first to block->next, otherwise, we should set block->prev->next = block->next
// if removing block is not last node, which means block->next != NULL
// we should set block->next->prev = block->prev

// finally, do not forget to return allocation memory address, which is (void *)block + memory_size

void *remove_from_freelist(memory_t *block)
{
	memory_t *prevNode = block->prev;
	memory_t *nextNode = block->next;
	if (prevNode == NULL)
	{
		first = nextNode;
	}
	else
	{
		prevNode->next = nextNode;
	}

	if (nextNode != NULL)
	{
		nextNode->prev = prevNode;
	}

  free_mem -=(block->size+sizeof(memory_t));

	return (void *)block + memory_size;
}

// helper function:
// aiming to increase heap memory for the given size
// genearl idea:
// using sbrk to increase heap memory, sbrk will return the start address we increase
// also, when we get block, we need initialize it
// block->next = NULL
// block->prev = NULL
// block->size = size
// because this block may be free in the future and we may insert it into free list
// so we need to know its information

// and we finally should return allocation memory address, which is (void*) block + memory_size

// do not forget to update our total_memory, i.e. total_memory += size + memory_size

void *create_mem(size_t size)
{
	// corner case check
	if (size <= 0)
	{
		return NULL;
	}

	memory_t *block = (memory_t *)sbrk(size + memory_size);
	block->prev = NULL;
	block->next = NULL;
	block->size = size;
	total_memory += size + memory_size;
	return (void *)block + memory_size;
}

// helper function:
// aiming to free allocated memory in the heap, which means we should insert a given block into freelist
// general idea:
// because we are given a memory address
// so first, we need to get ptr_begin which contains info of prevOne and nextOne and size
// i.e. memory_t * ptr_begin = (memory_t*)((void*)ptr - memory_size);
// after getting the ptr_begin
// we can get info of prevOne and nextOne by doing iteration (because the ptr_begin not in freelist, its prev and next is NULL)
// initially, we set
// memory_t * prevOne = NULL
// memory_t * nextOne = first
// we do while loop
// while nextOne is not NULL
// we check if ptr_begin < nextOne, that means we found a valid position to insert free block, we break
// otherwise, we set prevOne = nextOne and update nextOne = nextOne->next

// and after doing so, we get prevOne and nextOne, so we can insert our free block by calling
// function "insert_into_freelist"

// after inserting, we should make sure the adj block should be merged
// so call function "merge"

void free_memory(void *ptr)
{
	memory_t *ptr_begin = (memory_t *)((void *)ptr - memory_size);
	memory_t *prevOne = NULL;
	memory_t *nextOne = first;
	while (nextOne != NULL)
	{
		if (ptr_begin < nextOne)
		{
			break;
		}
		prevOne = nextOne;
		nextOne = nextOne->next;
	}

  free_mem +=(ptr_begin->size+sizeof(memory_t));

	insert_into_freelist(prevOne, nextOne, ptr_begin);
	merge(ptr_begin);
}

// helper function:
// aiming to insert ptr_begin into our freelist between prevOne and nextOne
// general idea:

// case1: if both prevOne and nextOne is NULL, we will insert block into empty freelist
// ptr_begin->next = NULL;
// ptr_beigin->prev = NULL;
// first = ptr_begin;

// case2: if prevOne is NULL, which means we will insert block into the beginning
// ptr_begin->prev = NULL;
// ptr_begin->next = nextOne;
// nextOne->prev = ptr_begin;
// first = ptr_begin;

// case3: if nextOne is NULL, which means we will insert block into the ending
// ptr_begin->next = NULL;
// ptr_begin->prev = prevOne;
// prevOne->next = ptr_begin;

// case4: if both prevOne and nextOne is not NULL, means we insert block between prevOne and nextOne
// ptr_begin->next = nextOne;
// ptr_begin->prev = prevOne;
// prevOne->next = ptr_begin;
// nextOne->prev = ptr_begin;

void insert_into_freelist(memory_t *prevOne, memory_t *nextOne, memory_t *ptr_begin)
{
	if (prevOne == NULL && nextOne == NULL)
	{
		ptr_begin->next = NULL;
		ptr_begin->prev = NULL;
		first = ptr_begin;
	}
	else if (prevOne == NULL)
	{
		ptr_begin->prev = NULL;
		ptr_begin->next = nextOne;
		first = ptr_begin;
	}
	else if (nextOne == NULL)
	{
		ptr_begin->next = NULL;
		ptr_begin->prev = prevOne;
		prevOne->next = ptr_begin;
	}
	else
	{
		ptr_begin->next = nextOne;
		ptr_begin->prev = prevOne;
		nextOne->prev = ptr_begin;
		prevOne->next = ptr_begin;
	}
}

// helper function:
// aiming to merge current block with prev block and next block if they are adjencent
// general idea:
// first, we can get prev block and next block
// memory_t * prevOne = block->prev;
// memory_t * nextOne = block->next;
// then, we check prevOne adj relation with current block
// after, we check nextOne adj relation with current block
// the criteria to judge whether the current block is adj with prevOne or nextOne is
// eg for nextOne
// (void *)block + block->size + memory_size == (void *)nextOne

// eg for prevOne
// (void *)prevOne + prevOne->size + memory_size == (void*)block

// and when it meet this criteria
// we should merge it by updating size and relation

// eg for nextOne
// block->size += memory_size + nextOne->size;
// block->next = nextOne->next;
// and if nextOne->next is not NULL, we need to set nextOne->next->prev = block;

// eg for prevOne
// prevOne->size += memory_size + block->size;
// prevOne->next = block->next;
// and if block->next is not NULL we need set prevOne->next->prev = prevOne

void merge(memory_t *block)
{
	memory_t *prevOne = block->prev;
	memory_t *nextOne = block->next;

	if (nextOne != NULL)
	{
		if ((void *)block + block->size + memory_size == (void *)nextOne)
		{
			block->size += memory_size + nextOne->size;
			block->next = nextOne->next;
			if (nextOne->next != NULL)
			{
				block->next->prev = block;
			}
		}
	}

	if (prevOne != NULL)
	{
		if ((void *)prevOne + prevOne->size + memory_size == (void *)block)
		{
			prevOne->size += memory_size + block->size;
			prevOne->next = block->next;
			if (block->next != NULL)
			{
				prevOne->next->prev = prevOne;
			}
		}
	}
}

// Get total data segment size
unsigned long get_data_segment_size()
{
	return total_memory;
}

// Get total space of free and meta data
unsigned long get_data_segment_free_space_size()
{
	unsigned long res = 0;
	memory_t *cur = first;
	while (cur!=NULL)
	{
		res += cur->size + memory_size;
		cur = cur->next;
	}
	return res;
}


#define NUM_ITERS    100
#define NUM_ITEMS    10000

//#ifdef FF
#define MALLOC(sz) ff_malloc(sz)
#define FREE(p)    ff_free(p)
// #endif
// #ifdef BF
// #define MALLOC(sz) bf_malloc(sz)
// #define FREE(p)    bf_free(p)
// #endif


double calc_time(struct timespec start, struct timespec end) {
  double start_sec = (double)start.tv_sec*1000000000.0 + (double)start.tv_nsec;
  double end_sec = (double)end.tv_sec*1000000000.0 + (double)end.tv_nsec;

  if (end_sec < start_sec) {
    return 0;
  } else {
    return end_sec - start_sec;
  }
};


struct malloc_list {
  size_t bytes;
  int *address;
};
typedef struct malloc_list malloc_list_t;

malloc_list_t malloc_items[2][NUM_ITEMS];

unsigned free_list[NUM_ITEMS];


int main(int argc, char *argv[])
{
  int i, j, k;
  unsigned tmp;
  unsigned long data_segment_size;
  unsigned long data_segment_free_space;
  struct timespec start_time, end_time;

  srand(0);

  const unsigned chunk_size = 32;
  const unsigned min_chunks = 4;
  const unsigned max_chunks = 16;
  for (i=0; i < NUM_ITEMS; i++) {
    malloc_items[0][i].bytes = ((rand() % (max_chunks - min_chunks + 1)) + min_chunks) * chunk_size;
    malloc_items[1][i].bytes = ((rand() % (max_chunks - min_chunks + 1)) + min_chunks) * chunk_size;
    free_list[i] = i;
  } //for i

  i = NUM_ITEMS;
  while (i > 1) {
    i--;
    j = rand() % i;
    tmp = free_list[i];
    free_list[i] = free_list[j];
    free_list[j] = tmp;
  } //while


  for (i=0; i < NUM_ITEMS; i++) {
    malloc_items[0][i].address = (int *)MALLOC(malloc_items[0][i].bytes);
  } //for i


  //Start Time
  //clock_gettime(CLOCK_MONOTONIC, &start_time);

  for (i=0; i < NUM_ITERS; i++) {
    unsigned malloc_set = i % 2;
    for (j=0; j < NUM_ITEMS; j+=50) {
      for (k=0; k < 50; k++) {
	unsigned item_to_free = free_list[j+k];
	FREE(malloc_items[malloc_set][item_to_free].address);
      } //for k
      for (k=0; k < 50; k++) {
	malloc_items[1-malloc_set][j+k].address = (int *)MALLOC(malloc_items[1-malloc_set][j+k].bytes);
      } //for k
    } //for j
  } //for i

  //Stop Time
  //clock_gettime(CLOCK_MONOTONIC, &end_time);

  data_segment_size = get_data_segment_size();
  data_segment_free_space = get_data_segment_free_space_size();
  printf("data_segment_size = %lu, data_segment_free_space = %lu\n", data_segment_size, data_segment_free_space);

  double elapsed_ns = calc_time(start_time, end_time);
  printf("Execution Time = %f seconds\n", elapsed_ns / 1e9);
  printf("Fragmentation  = %f\n", (float)data_segment_free_space/(float)data_segment_size);



  for (i=0; i < NUM_ITEMS; i++) {
    FREE(malloc_items[0][i].address);
  } //for i

  return 0;
}
