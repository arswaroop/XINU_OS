#include <xinu.h>
#include <stdio.h>
#include <segmem.h>

shellcmd xsh_segmem(int nargs, char *args[])
{
	xmalloc_init() ;
	void* ptr = xmalloc(10) ;
	xheap_snapshot();
	printf("%s\n",xheap_snapshot());
	xfree(ptr);
	char* temp = xheap_snapshot();
	printf("%s\n",temp);
	return OK;
}
