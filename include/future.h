#ifndef _FUTURE_H_
#define _FUTURE_H_  

typedef struct future_qnode{
	pid32  pid;
	int set_val;
	struct future_qnode* next;
	struct future_qnode* prev;
}future_qnode;
typedef struct future_qnode* future_q;


typedef enum {
  FUTURE_EMPTY,
  FUTURE_WAITING,
  FUTURE_READY
} future_state_t;

typedef enum {
  FUTURE_EXCLUSIVE,
  FUTURE_SHARED,
  FUTURE_QUEUE
} future_mode_t;



typedef struct {
  int value;
  future_state_t state;
  future_mode_t mode;
  pid32 pid;
  future_q set_queue;
  future_q get_queue;
} future_t;

// Queue functions
future_q new_q();
int fq_isempty(future_q);
int future_enqueue(future_q head, pid32 pid);
pid32 future_dequeue(future_q head);
int free_q(future_q head);
/* Interface for the Futures system calls */
future_t* future_alloc(future_mode_t mode);
syscall future_free(future_t*);
syscall future_get(future_t*, int*);
syscall future_set(future_t*, int);

// Future producer and consumer 
uint future_prod(future_t* ,int);
uint future_cons(future_t*); 
#endif /* _FUTURE_H_ */
