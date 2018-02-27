#include <sys/types.h>

/* scheduler Header file scheduler.h*/
#define current_thread (get_current_thread())
/* enum to maintain state of thread  */ 
typedef enum {
	RUNNING,
	READY,
	BLOCKED,
	DONE
} state_t;

/* scheduler API functions to implement */
void scheduler_begin();
void scheduler_end();
void thread_fork(void(*target)(void*), void * arg);
void thread_finish();
void yield();
ssize_t read_wrap(int fd, void * buf, size_t count);

/* thread structure */
struct thread_t

{
	void *sp; // stock pointer
	void (*initial_function) (void*); // function pointer
	void *initial_arg; // intial arguments
	state_t state; // thread state
};
/* Mutex Structure */
struct mutex    
{               
	struct thread_t *locked_thread;
	struct queue *waiting_queue;
};
/* Condition mutex */
struct condition
{
	struct queue *waiting_con_queue;
};
extern struct thread_t * get_current_thread();
extern void set_current_thread(struct thread_t*);
