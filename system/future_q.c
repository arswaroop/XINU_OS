#include <xinu.h>
#include <future.h>
future_q new_q()
{
	//printf("Inside new_q\ni>>>>>>>>>>>");
	future_q head = (future_q)getmem(sizeof(struct future_qnode));
	future_q tail = (future_q)getmem(sizeof(struct future_qnode));
	
	head->set_val = MAXKEY;
	tail->set_val = MINKEY;
	head->next = tail;
	head->prev = NULL;
	tail->next = NULL;
	tail->prev = head;
	return  head;	
}

int fq_isempty(future_q q)
{
	if(q == NULL)
	{
		return SYSERR;
	}
//	printf("Inside fq_isempty: set_val:%d\n" ,q->next->set_val );
	return (q->next->set_val == MINKEY);
}

int future_enqueue(future_q head, pid32 pid)
{
	if (head == NULL)
	{
		return SYSERR;
	}
	
	future_q curr = head;
	while (curr->next->set_val != MINKEY)
	{
		curr = curr->next;
	}
	future_q prev = curr;
	future_q last = curr->next;

	// New node
	prev->next = (future_q)getmem(sizeof(struct future_qnode));

	curr = prev->next;

	curr->pid = pid;	
	curr->next = last;
	curr->prev = prev;
	
	last->prev = curr;
	return OK;
}

pid32 future_dequeue(future_q head)
{
	if (head == NULL || fq_isempty(head))
	{
		return (pid32)SYSERR;
	}
	
	future_q node = head->next;
	head->next = node->next;
	node->next->prev = head;
	pid32 pid = node->pid;
	freemem((char*)node,(sizeof(struct future_qnode)));
	//printf("Memory freed\n>>>>>>>>>>>>>>\n");	
	return pid; 
	
}
int free_q(future_q head)
{
	if (head == NULL || !(fq_isempty(head)))
	{
		return (int)SYSERR;	
	}
	future_q tail = head->next;
	freemem((char*)head,(sizeof(struct future_qnode)));
	freemem((char*)tail,(sizeof(struct future_qnode)));
	return (int)OK;
}
