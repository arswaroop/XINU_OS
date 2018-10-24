#include <xinu.h>
#include <segmem.h>
#include <string.h>
#include <stdio.h>

#define BUF_SIZE 50

bpid32 buffers[NUM_BUFFERS][4];

void xmalloc_init()
{
	int i = 0;
	for(i=1; i<=NUM_BUFFERS; i++)
	{
		leftover_list head = (leftover_list)getmem(sizeof(struct leftover_block));
		head->next = NULL;
		head->bufptr = 0;
		head->size = 0;
		buffers[i-1][0] = mkbufpool(32*i, BUF_SIZE);
		buffers[i-1][1] = 0; // Allocated bytes
		buffers[i-1][2] = 0; // Allocated buffers
		buffers[i-1][3] = (uint32) head; // Pointer to linked list
		if (buffers[i-1][0] == (bpid32)SYSERR)
		{
			printf("An error occured while allocating buffer!\n");
		}
	}
}

void* xmalloc(int size) 
{
	if (size < 0)
	{	
		printf("Size not valid\n");
		return (void*) SYSERR;
	}
	int i;
	for (i =1; i<= NUM_BUFFERS; i++)
	{
		if(size <= 32* i)
		{
			// Check if buffer empty
			if (buffers[i-1][2] > 10)
			{
				// Do something
			}
			struct bpentry* buf = (struct bpentry*)getbuf(buffers[i-1][0]);
			if(buf == (void*)SYSERR)
			{
				printf("Error while fetching memory of size %d\n",size);
				return (void*) SYSERR;
			}
			buffers[i-1][1] += size;
			buffers[i-1][2] += 1;
			leftover_list leftover = (leftover_list)getmem(sizeof(struct leftover_block));
			leftover->next = ((leftover_list)buffers[i-1][3])->next;
			leftover->bufptr =(uint32) buf;
			leftover->size =(uint32) 32*i-size;
			((leftover_list)buffers[i-1][3])->next = leftover;
			return (void *)buf;
		}
	}	
	printf("The size is bigger than any buffer pool\n");
	return (void*) SYSERR;
}

void xfree(void* ptr) 
{	void * temp_addr = ptr - sizeof(bpid32);
	bpid32 poolid = *(bpid32 *)temp_addr;
	
	int i =0;
	for (i=0; i< NUM_BUFFERS; i++)
	{
		if (poolid == (bpid32)buffers[i][0])
		{	
			leftover_list prev = (leftover_list)buffers[i][3];
			leftover_list leftover = prev->next;
			while(leftover != NULL)
			{
				if ((void*)leftover->bufptr == ptr)
				{
					buffers[i][1] -= (32*(i+1) - leftover->size);
					buffers[i][2] -= 1;
					prev->next = leftover->next;
					if (freebuf((void*)ptr) == SYSERR)
					{
						printf("An error occured while freeing memory \n");
					}
					//printf("Size is %d\n",buffers[i][1]);
					return;
				}
				prev = leftover;
				leftover = leftover->next;
			}
		}
	}
}
char* xheap_snapshot()
{
	char result[1024 * NUM_BUFFERS];
	int pointer=0;
		int i;
	for(i=0; i< NUM_BUFFERS; i++)
	{
		char buffer[1024];
		sprintf(buffer, "pool_id=%d,buffer_size=%d, total_buffers=%d, allocated_bytes=%d, allocated_buffers=%d, fragmented_bytes=%d \n",buffers[i][0], 32*(i+1), NUM_BUFFERS, buffers[i][1],buffers[i][2], 32*(i+1)*buffers[i][2] -buffers[i][1]);
		int j = 0;
		while (buffer[j] != '\0')
		{
			result[pointer++]=buffer[j++];
		}
		
	}
	//printf("%s", result);
	return result; 
}
