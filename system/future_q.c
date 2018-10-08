#include <xinu.h>
#include <future.h>

// This function allocates a memory for the queue and returns the pointer to the first node.
future_q new_q()
{
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
// Checks for an empty queue by checking ig next node is the tail node.
int fq_isempty(future_q q)
{
	if(q == NULL)
	{
		return SYSERR;
	}
	return (q->next->set_val == MINKEY);
}

// Enqueue a node in the given queue
int future_enqueue(future_q head, pid32 pid)
{
	if (head == NULL)
	{
		return SYSERR;
	}
	
	future_q curr = head;
	// Look for the last node and insert between two nodes
	while (curr->next->set_val != MINKEY)
	{
		curr = curr->next;
	}
	future_q prev = curr;
	future_q last = curr->next;

	prev->next = (future_q)getmem(sizeof(struct future_qnode));

	curr = prev->next;

	curr->pid = pid;	
	curr->next = last;
	curr->prev = prev;
	
	last->prev = curr;
	return OK;
}

// This function dequeues the first element of the queue
pid32 future_dequeue(future_q head)
{
	// Checks null conditions
	if (head == NULL || fq_isempty(head))
	{
		return (pid32)SYSERR;
	}
		
	future_q node = head->next;
	head->next = node->next;
	node->next->prev = head;
	pid32 pid = node->pid;

	//Free the memory block occupied by current node
	freemem((char*)node,(sizeof(struct future_qnode)));
	return pid; 
	
}
// Frees head and tail nodes to free up space
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
