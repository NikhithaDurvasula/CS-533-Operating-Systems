
/* scheduler API Program scheduler.c */
#include <stdarg.h>
#define _GNU_SOURCE
#include <sched.h> 
#include<stdio.h>
#include<stdlib.h>
#include <atomic_ops.h>
#include "queue.h"   // including queue header file
#include "scheduler.h"	// including scheduler header
AO_TS_t spinlock1 = AO_TS_INITIALIZER;
struct queue *ready_queue;
void spinlock_lock(AO_TS_t * lock) 
{               
	while(AO_test_and_set_acquire(lock)==AO_TS_SET)
	{

	}
}       
void spinlock_unlock(AO_TS_t * lock) 
{
	AO_CLEAR(lock); 
}

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

void mutex_init(struct mutex *mutexvar)
{
	mutexvar->waiting_queue=malloc(sizeof(struct queue *)); // allocating  memory for waiting queue for givem mutex
	mutexvar->waiting_queue->head=NULL;                                 // intializing head to NULL
	mutexvar->locked_thread = NULL;			// intializing locked thread to NULL
}
/* function to Lock based on mutex */
void mutex_lock(struct mutex *mutexvar)
{
	if(mutexvar->locked_thread==NULL)	/* checking if there is any thread already locked */
	{		
		mutexvar->locked_thread=current_thread;	/* if nothing locked locked current thread */
	}	
	/* if there is a thread already in locked state keep current thread in blocked state and put it in waiting queue of the mutex*/
	else
	{ 
		current_thread->state=BLOCKED;	
		thread_enqueue((mutexvar->waiting_queue),current_thread);
		yield(); /* yield to next thread */
	}
}
/* function to unlock the current thread if any locked thread exists and on given mutex and keeping it in to ready queue */
void mutex_unlock(struct mutex *mutexvar)
{
	if(mutexvar->locked_thread==current_thread)	/* checking current thread and locked hread which is trying to unlock are the same */
	{
		mutexvar->locked_thread=NULL;	/* making locked tread to NULL */
		if(is_empty(mutexvar->waiting_queue)!=1)	/* checking the there is any thread waiting for the lock */
		{
			/* chaging status of waiting thread to READy and giving lock to it and keeping it in ready queue */
			struct thread_t *new_thread=thread_dequeue(mutexvar->waiting_queue); 			
			new_thread->state=READY;
			mutexvar->locked_thread=new_thread;
			thread_enqueue(ready_queue,new_thread);
		}
	}
	else
		printf("error wrong thread trying to unlock");
}

int kernel_thread_begin(void *arg)
{       
	set_current_thread(malloc(sizeof(struct thread_t)));    // Allocating memory for current_thread
	current_thread->state=RUNNING;
	set_current_thread(current_thread);
	while(1)
		yield();
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
	spinlock_lock(&spinlock1);
	struct thread_t *new_thread= malloc(sizeof(struct thread_t));  // Creating and allocating new thread
	new_thread->sp = (void *) malloc(60000) + 60000;   		//allocating memory for stack pointer for new thread
	new_thread->initial_function=target; 				// intializing intial_function to function target function passed
	new_thread->initial_arg=arg;					// assigging intial_arg with passed argument
	new_thread->state=RUNNING;					// intialing new thread state to RUNNING
	current_thread->state=READY;					// keeping old thread to READY state
	thread_enqueue(ready_queue, current_thread);			//pushing current_thread to ready_queue
	struct thread_t *temp = current_thread;   
	set_current_thread(new_thread);
	thread_start(temp, current_thread);		// calling thread start assembly to create new thread
	spinlock_unlock(&spinlock1);
}

/*Yield function to check state and switch threads in queue */
void yield()
{
	spinlock_lock(&spinlock1);
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

/* Scheduler end to yiels to all threads till all threads complete */
void scheduler_end()
{
	spinlock_lock(&spinlock1);
	while(is_empty(ready_queue)!=1)    // checking queue is empty
	{
		spinlock_unlock(&spinlock1);
		yield();       // yield to the next thread in queue
		spinlock_lock(&spinlock1);
	}
	spinlock_unlock(&spinlock1);
}

void thread_wrap()
{
	spinlock_unlock(&spinlock1);
	current_thread->initial_function(current_thread->initial_arg);
	current_thread->state=DONE;
	yield();
}

