/* scheduler API Program scheduler.c */
#include <stdarg.h>
#define _GNU_SOURCE
#include <sched.h> 
#include<stdio.h>
#include<stdlib.h>
#include <atomic_ops.h>
#include "queue.h"   // including queue header file
#include "scheduler.h"	// including scheduler header
AO_TS_t spinlock1 = AO_TS_INITIALIZER;	// Ready list spin lock
AO_TS_t spinlock_mutex = AO_TS_INITIALIZER;		// mutex spinlock
void spinlock_lock(AO_TS_t * lock) ;		
void spinlock_unlock(AO_TS_t * lock);
struct queue *ready_queue;


#undef malloc
#undef free
void * safe_mem(int op, void * arg) {
	static AO_TS_t spinlock = AO_TS_INITIALIZER;
	void * result = 0;

	spinlock_lock(&spinlock);
	if(op == 0) {
		result = malloc((size_t)arg);
	} else 
	{
		free(arg);
	}
	spinlock_unlock(&spinlock);
	return result;
}
#define malloc(arg) safe_mem(0, ((void*)(arg)))
#define free(arg) safe_mem(1, arg)


// Spin lock implementation
void spinlock_lock(AO_TS_t * lock) 
{               
	while(AO_test_and_set_acquire(lock))
	{	
	}
}  

// Spin un lock implementation     
void spinlock_unlock(AO_TS_t * lock) 
{
	AO_CLEAR(lock); 
}

// New Kernel thread creation
int kernel_thread_begin(void *arg)
{        

	set_current_thread(malloc(sizeof(struct thread_t)));    // Allocating memory for current_thread
	current_thread->state=RUNNING;				// Setting state to running
	set_current_thread(current_thread);			
	while(1)
		yield();
}
/* works exactly as yield but used to make sure thread in locked state till completion */
void block(AO_TS_t * spinlock)
{
	spinlock_lock(&spinlock1);		// locking ready list 	
	spinlock_unlock(spinlock);		// unlocking mutex variable
	if(current_thread->state!=DONE && current_thread->state!=BLOCKED )			// checking state of current thread
	{
		current_thread->state=READY;		// if not done keeping it to Ready
		thread_enqueue(ready_queue,current_thread);	//pushing thread to ready_queue
	}
	struct thread_t *new_thread=thread_dequeue(ready_queue);	 // creating new_thread with next thread in queue
	struct thread_t *temp =current_thread;
	set_current_thread(new_thread);
	thread_switch(temp, current_thread);		// Switching to new_thread
	spinlock_unlock(&spinlock1);		

}

/* function to intialize mutex variable */
void mutex_init(struct mutex *mutexvar)
{
	mutexvar->waiting_queue=malloc(sizeof(struct queue *)); // allocating  memory for waiting queue for givem mutex
	mutexvar->waiting_queue->head=NULL;                                 // intializing head to NULL
	mutexvar->locked_thread = NULL;			// intializing locked thread to NULL
}
/* function to Lock based on mutex */
void mutex_lock(struct mutex *mutexvar)
{
	spinlock_lock(&spinlock_mutex);// lock mutex
	if(mutexvar->locked_thread==NULL)	/* checking if there is any thread already locked */
	{		
		mutexvar->locked_thread=current_thread;	/* if nothing locked locked current thread */
	}	
	/* if there is a thread already in locked state keep current thread in blocked state and put it in waiting queue of the mutex*/
	else
	{
		current_thread->state=BLOCKED;	
		thread_enqueue((mutexvar->waiting_queue),current_thread);
		block(&spinlock_mutex); /* yield to next thread */
		spinlock_lock(&spinlock_mutex);
	}
	spinlock_unlock(&spinlock_mutex);// unlock mutex
}
/* function to unlock the current thread if any locked thread exists and on given mutex and keeping it in to ready queue */
void mutex_unlock(struct mutex *mutexvar)
{
	spinlock_lock(&spinlock_mutex);	// lock mutex
	if(mutexvar->locked_thread==current_thread)	/* checking current thread and locked hread which is trying to unlock are the same */
	{
		mutexvar->locked_thread=NULL;	/* making locked tread to NULL */
		if(is_empty(mutexvar->waiting_queue)!=1)	/* checking the there is any thread waiting for the lock */
		{
			/* chaging status of waiting thread to READy and giving lock to it and keeping it in ready queue */
			struct thread_t *new_thread=thread_dequeue(mutexvar->waiting_queue); 			
			spinlock_lock(&spinlock1);
			new_thread->state=READY;
			mutexvar->locked_thread=new_thread;

			thread_enqueue(ready_queue,new_thread);
			spinlock_unlock(&spinlock1);
		}
	}
	else
		printf("error wrong thread trying to unlock");

	spinlock_unlock(&spinlock_mutex);	// unlock mutex

}


/* Scheduler begin function to create ready queue */ 
void scheduler_begin()
{
	ready_queue=malloc(sizeof(struct queue *)); // allocating ready_queue memory
	ready_queue->head=NULL; 				// intializing head to NULL
	set_current_thread(malloc(sizeof(struct thread_t))); 	// Allocating memory for current_thread
	char  * child_stacks;
	child_stacks = malloc(4096) + 4096;
	clone(kernel_thread_begin,child_stacks, CLONE_THREAD|CLONE_VM|CLONE_SIGHAND|CLONE_FILES|CLONE_FS|CLONE_IO,NULL);
}

/* thread_fork function to allocate memory for new thread and start new thread  */
void thread_fork(void(*target)(void*), void * arg)
{
	//	spinlock_lock(&spinlock1);	// lock ready list
	struct thread_t *new_thread= malloc(sizeof(struct thread_t));  // Creating and allocating new thread
	new_thread->sp = (void *) malloc(100000000) + 100000000;   		//allocating memory for stack pointer for new thread
	new_thread->initial_function=target; 				// intializing intial_function to function target function passed
	new_thread->initial_arg=arg;					// assigging intial_arg with passed argument
	new_thread->state=RUNNING;					// intialing new thread state to RUNNING
	current_thread->state=READY;
	spinlock_lock(&spinlock1);					// keeping old thread to READY state
	thread_enqueue(ready_queue, current_thread);			//pushing current_thread to ready_queue
	struct thread_t *temp = current_thread;   
	set_current_thread(new_thread);
	thread_start(temp, current_thread);		// calling thread start assembly to create new thread
	spinlock_unlock(&spinlock1);	// unlock ready list
}

/*Yield function to check state and switch threads in queue */
void yield()
{
	spinlock_lock(&spinlock1);	// lock ready list
	if(current_thread->state!=DONE && current_thread->state!=BLOCKED )			// checking state of current thread
	{
		current_thread->state=READY;		// if not done keeping it to Ready
		thread_enqueue(ready_queue,current_thread);	//pushing thread to ready_queue
	}
	struct thread_t *new_thread=thread_dequeue(ready_queue);	 // creating new_thread with next thread in queue
	struct thread_t *temp =current_thread;
	set_current_thread(new_thread);
	thread_switch(temp, current_thread);		// Switching to new_thread
	spinlock_unlock(&spinlock1);	// unlock ready list
}

/* Scheduler end to yiels to all threads till all threads complete */
void scheduler_end()
{
	spinlock_lock(&spinlock1);	// lock ready list
	while(is_empty(ready_queue)!=1)    // checking queue is empty
	{
		spinlock_unlock(&spinlock1);
		yield();       // yield to the next thread in queue
		spinlock_lock(&spinlock1);
	}
	spinlock_unlock(&spinlock1);	// unlock ready list
}

// Function to unlock and run user thread
void thread_wrap()
{
	spinlock_unlock(&spinlock1);		// unlock ready list
	current_thread->initial_function(current_thread->initial_arg);
	current_thread->state=DONE;
	yield();
}

