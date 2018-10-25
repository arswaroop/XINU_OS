#ifndef _SEGMEM_H_
#define _SEGMEM_H_

#define NUM_BUFFERS 11 

// Structure to save fragmented memory
typedef struct leftover_block{
	uint32 bufptr;
	uint32 size;
	struct leftover_block* next;
}leftover_block;

// Creating a variable to leftover list
typedef struct leftover_block* leftover_list;

// Initializing an array to save the buffer stats
extern bpid32 buffers[NUM_BUFFERS][4];


void xmalloc_init() ;
void*  xmalloc(int size) ;
void xfree(void* ptr) ;
const char* xheap_snapshot();
#endif
