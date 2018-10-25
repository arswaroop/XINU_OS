#include <xinu.h>
#include <segmem.h>
#include <string.h>
#include <stdio.h>

#define BUF_SIZE 50

// Initialize an array to save 
// 1. PoolID
// 2. Allocated bytes
// 3. Number of allocated buffers
// 4. Pointer to the linked list for free buff
bpid32 buffers[NUM_BUFFERS][4];

void xmalloc_init()
{
	int i = 0;
	int shift = 1;
	// Initializing the buffer array to store the above mentioned values.
	for(i=1; i<=NUM_BUFFERS; i++)
	{
		leftover_list head = (leftover_list)getmem(sizeof(struct leftover_block));
		head->next = NULL;
		head->bufptr = 0;
		head->size = 0;
		
		buffers[i-1][0] = mkbufpool( 8 *shift, BUF_SIZE);
		buffers[i-1][1] = 0; // Allocated bytes
		buffers[i-1][2] = 0; // Allocated buffers
		buffers[i-1][3] = (uint32) head; // Pointer to linked list
		if (buffers[i-1][0] == (bpid32)SYSERR)
		{
			printf("An error occured while allocating buffer!\n");
		}
		shift *= 2;
	}
}
// Function to allocate memory from appropriate buffer
void* xmalloc(int size) 
{
	if (size < 0)
	{	
		printf("Size not valid\n");
		return (void*) SYSERR;
	}
	int i;
	int shift = 1;
	for (i =1; i<= NUM_BUFFERS; i++)
	{
		if(size <=  8 * shift)
		{
			// Check if buffer empty
			if (buffers[i-1][2] >= BUF_SIZE)
			{
				shift *= 2;
				continue;
			}
			// Call getbuf() to allocate the memory from given bufferpool
			struct bpentry* buf = (struct bpentry*)getbuf(buffers[i-1][0]);
			if(buf == (void*)SYSERR)
			{
				printf("Error while fetching memory of size %d\n",size);
				return (void*) SYSERR;
			}
			buffers[i-1][1] += size;
			buffers[i-1][2] += 1;
			// Store the leftover of the buffer in a linked list
			leftover_list leftover = (leftover_list)getmem(sizeof(struct leftover_block));
			leftover->next = ((leftover_list)buffers[i-1][3])->next;
			leftover->bufptr =(uint32) buf;
			leftover->size =(uint32) 8*shift -size;
			((leftover_list)buffers[i-1][3])->next = leftover;
			return (void *)buf;
		}
		shift *= 2;
	}	
	printf("The memory of size %d is not available in any pool at this moment\n",size);
	return (void*) SYSERR;
}

// Function to free the given address
void xfree(void* ptr) 
{	void * temp_addr = ptr - sizeof(bpid32);
	bpid32 poolid = *(bpid32 *)temp_addr; // Get the buffer poolid og the pointer
	
	int i =0;
	int shift = 1;
	for (i=0; i< NUM_BUFFERS; i++)
	{	
		// Check for valid pool-id
		if (poolid == (bpid32)buffers[i][0])
		{	
			// Iterate over leftover list to check if there is any fragmentation
			leftover_list prev = (leftover_list)buffers[i][3];
			leftover_list leftover = prev->next;
			while(leftover != NULL)
			{
				// Change the allocated size and free the node from linked list
				if ((void*)leftover->bufptr == ptr)
				{
					buffers[i][1] -= (8*shift - leftover->size);
					buffers[i][2] -= 1;
					prev->next = leftover->next;
					if (freebuf((void*)ptr) == SYSERR)
					{
						printf("An error occured while freeing memory \n");
					}
					return;
				}
				prev = leftover;
				leftover = leftover->next;
			}
		}
		shift *=2;
	}
	printf("The pointer could not be found in the buffer pools\n");
}
// Function for snapshot of memory
const char* xheap_snapshot()
{
	char result[1024 * NUM_BUFFERS];
	int pointer=0;
	int i;
	int shift = 1;
	// Check the local array for buffer stats
	for(i=0; i< NUM_BUFFERS; i++)
	{
		char buffer[1024];
		sprintf(buffer, "pool_id=%d,buffer_size=%d, total_buffers=%d, allocated_bytes=%d, allocated_buffers=%d, fragmented_bytes=%d \n",buffers[i][0], 8*shift,BUF_SIZE, buffers[i][1],buffers[i][2], 8* shift *buffers[i][2] -buffers[i][1]);
		int j = 0;
		while (buffer[j] != '\0')
		{
			result[pointer++]=buffer[j++];
		}
		shift *=2;
	}
	return result; 
}
