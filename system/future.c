#include <xinu.h>
#include <future.h>


// Allocation of memory block of type future_t
future_t* future_alloc(future_mode_t mode)
{
	future_t* a =(future_t*)(getmem(sizeof(future_t)));
	a->state 	= FUTURE_EMPTY;
	a->mode		= mode;
	
	if (mode == FUTURE_EXCLUSIVE);	// Exclusive mode has no queues

	if(mode == FUTURE_SHARED)	// Allocate a get_queue for SHARED MODE
	{
		a->get_queue	= (future_q)new_q();
	}
	// Allocate get_queue and set_queue for Future mode
	else if(mode == FUTURE_QUEUE)	 
	{
		a->set_queue	= (future_q)new_q();
		a->get_queue	= (future_q)new_q();
	}
	
	return a;
}

// Free up all the space assiciated with future including the queues
syscall future_free(future_t* f)	
					
{
	if(f == NULL )
	{
		return SYSERR;
	}
	if (f->mode == FUTURE_EXCLUSIVE)
	{	
		return OK;
	}
	// Check for valid queues in SHARED and QUEUE mode
	else if( (f->mode == FUTURE_SHARED && free_q(f->get_queue) )||\
		( f->mode == FUTURE_QUEUE && free_q(f->set_queue) && free_q(f->get_queue)  ))
	{
		freemem((char*)f, sizeof(future_t));
		return OK;
	}
	
	return SYSERR;
	
}

// Get the value of a future set by an operation and may change the state of future
syscall future_get(future_t* f, int* value)
{
	if (f == NULL) 	{
		return SYSERR;	
	}
	/////// GET>EXCLUSIVE/ Check conditions for exclusive mode 
	if(f->mode == FUTURE_EXCLUSIVE)		
	{
		/////// GET>EXCLUSIVE>WAITING/ Exclusive mode can have just one waiting future
		if( f->state == FUTURE_WAITING)	
		{
			return SYSERR;
		}
		/////// GET>EXCLUSIVE>EMPTY// The consumer needs to wait for the producer in case it is called first
		else if (f->state == FUTURE_EMPTY)
		{
			f->pid = getpid();
			f->state = FUTURE_WAITING;
			suspend(f->pid);
			*value =(int) f->value;
		}
		/////GET>EXCLUSIVE>READY// In case producer is ready, consumer prints the output
		else if (f->state == FUTURE_READY)	
		{
			*value = (int) f->value;
		}
	}

	////// GET>SHARED // Has N consumers and 1 prodicer. // Consumers can wait in a queue
	else if (f->mode == FUTURE_SHARED)	
	{
		////// GET>SHARED>EMPTY // Adds a new element and changes the state to waiting
		if (f->state == FUTURE_EMPTY || f->state == FUTURE_WAITING)
		{
			f->state = FUTURE_WAITING;
			future_enqueue(f->get_queue, getpid());
			suspend(getpid());
			*value =(int) f->value;

		}
		// GET>SHARED>READY // Allocates a value to all the consumers in the queue
		else if (f->state == FUTURE_READY)
		{
//			pid32 pid;
//			while ((pid = future_dequeue(f->get_queue)))
//			{
//				resume(pid);
//			}
			*value =(int) f->value;
			//future_free(f);	
		}
	}
	// GET>>QUEUE // Queues consumers and producers	
	else if (f->mode == FUTURE_QUEUE)
	{
		// Check if any producers are ready
		// If no producers are ready with results, consumers are enqueued		
		if (fq_isempty(f->set_queue))
		{
			future_enqueue(f->get_queue,getpid());
			suspend(getpid());
			*value =(int) f->value;
		}
		// If producers are ready with result, enqueued producer assigns a value to future and consumer consumes it.
		else
		{
			pid32 pid = future_dequeue(f->set_queue);
			resume(pid);
			*value = (int)f->value; 
		}
	}	

	return OK;

}

syscall future_set(future_t* f, int value)
{
	// Check for null
	if(f == NULL )
	{
		return SYSERR;
	}
	///// SET>EXCLUSIVE // Exclusive producer can have just one producer at a time
	if (f->mode == FUTURE_EXCLUSIVE)
	{
		///// SET>EXCLUSIVE>READY // If already another producer is ready, an error must be thrown
		if ( f->state == FUTURE_READY)
		{
			return SYSERR;
		}
		// SET>>EXCLUSIVE>>WAITING // A waiting consumer reads the value set in future after resume() call
		else if( f->state == FUTURE_WAITING)
		{	
			f->value = value;
			f->state = FUTURE_READY;
			resume(f->pid);
		}
		// SET>>EXCLUSIVE>>EMPTY // An empty future state required the value and changes the state 
		else if( f->state == FUTURE_EMPTY)
		{
			f->value = value;
			f->state = FUTURE_READY;
		}

	}
	// SET>>SHARED // Shared mode allows multiple consumers and 1 producer 
	if (f->mode == FUTURE_SHARED)
	{
		// SET>>SHARED>READY // If the value has been produced, it can not be set again by another producer, hence error.
		if (f->state == FUTURE_READY)
		{
			printf("Invalid produce!\n");
			//suspend(getpid());
			return SYSERR;
		}
		// SET>>SHARED>>EMPTY // If the future is empty, the value can be set and state can be changed to ready.
		else if (f->state == FUTURE_EMPTY)
		{
			f->value = value;
			f->state = FUTURE_READY;
		}
		// SET>>SHARED>>WAITING // In this case, all the processes in waiting should be resumed to read the value in future.
		// This is achieved using a while loop to resume process until the queue is vacant.
		else if (f->state == FUTURE_WAITING)
		{
			f->state = FUTURE_READY;
			f->value = value;
			pid32 pid;
                        while ((pid = future_dequeue(f->get_queue)) != (pid32)SYSERR )
                        {
                                resume(pid);
                        }
			//future_free(f);
		}
	}
	// SET>> QUEUE // This variation can handle multiple produces and consumers
	if (f->mode == FUTURE_QUEUE)
	{	
		// CHECKS FOR NULL QUEUE
		if (f->get_queue == NULL)
		{
			return SYSERR;
		}
		// There are no modes in this 
		if (fq_isempty(f->get_queue))
		{
			pid32  pid;
			pid = getpid();
			future_enqueue(f->set_queue, getpid());
			suspend(pid);
			f->value = value;
			return OK;
		}
		else
		{
			f->value = value;
			pid32 pid;
			pid = future_dequeue(f->get_queue);
			resume(pid);
		}
	}	
	return OK;


}








