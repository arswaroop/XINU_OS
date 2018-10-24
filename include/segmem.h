#ifndef _SEGMEM_H_
#define _SEGMEM_H_

#define NUM_BUFFERS 10 

/*typedef struct buf_leftover_block{
	uint32 curr;
	uint32 leftover;
	(struct buff_leftover_block*) next;
}buf_leftover_block;*/
typedef struct leftover_block{
	uint32 bufptr;
	uint32 size;
	struct leftover_block* next;
}leftover_block;
typedef struct leftover_block* leftover_list;
//extern struct leftover_block* head;
extern bpid32 buffers[NUM_BUFFERS][4];
//extern (struct buf_leftover_block*) leftover_list[NUM_BUFFERS];
//extern (struct memblk *) leftover_list;
// segmem.c methods
void xmalloc_init() ;
void*  xmalloc(int size) ;
void xfree(void* ptr) ;
char* xheap_snapshot();
#endif
