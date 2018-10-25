#include <xinu.h>
#include <stdio.h>
#include <segmem.h>

shellcmd xsh_segmem(int nargs, char *args[])
{
	xmalloc_init() ;
	void* ptr = xmalloc(10) ;
	printf("%s\n",xheap_snapshot());
	int i;
	void *ptrs[10] ;
	//= (void*)malloc(52 * sizeof(uint32));	
	for (i=0; i<10; i++)
	{
	        ptrs[i] = xmalloc(25*i) ;
		//printf("%s\n",xheap_snapshot());
	}
	for (i=0; i< 10; i++)
	{	
		xfree(ptrs[i]);
		//printf("%s\n",xheap_snapshot());
	}
	xfree(ptr);
	char* temp = xheap_snapshot();
	printf("%s\n",temp);
	return OK;
}
