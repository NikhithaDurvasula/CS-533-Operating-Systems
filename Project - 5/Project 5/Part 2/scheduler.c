
/* scheduler API Program scheduler.c */
#define _GNU_SOURCE
#include <sched.h> 
#include<stdio.h>
#include<stdlib.h>
#include <atomic_ops.h>
#include "queue.h"   // including queue header file
#include "scheduler.h"	// including scheduler header

struct queue *ready_queue;

// Spin lock Implementation
void spinlock_lock(AO_TS_t * lock) 
{               
	while(AO_test_and_set_acquire(lock)==AO_TS_SET)
	{
	}
}

// Spin un lock Implementation    
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
	} else {
		free(arg);
	}
	spinlock_unlock(&spinlock);
	return result;
}
#define malloc(arg) safe_mem(0, ((void*)(arg)))
#define free(arg) safe_mem(1, arg)

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
	set_current_thread(malloc(sizeof(struct thread_t))); 	// Allocating memory for current_thread
	char  * child_stacks;
	child_stacks = malloc(4096) + 4096;
	clone(kernel_thread_begin,child_stacks, CLONE_THREAD|CLONE_VM|CLONE_SIGHAND|CLONE_FILES|CLONE_FS|CLONE_IO,NULL);
}

/* thread_fork function to allocate memory for new thread and start new thread  */
void thread_fork(void(*target)(void*), void * arg)
{
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
}

/*Yield function to check state and switch threads in queue */
void yield()
{
	if(current_thread->state!=DONE && current_thread->state!=BLOCKED )			// checking state of current thread
	{
		current_thread->state=READY;		// if not done keeping it to Ready
		thread_enqueue(ready_queue,current_thread);	//pushing thread to ready_queue
	}
	struct thread_t *new_thread=thread_dequeue(ready_queue);	 // creating new_thread with next thread in queue
	struct thread_t *temp =current_thread;
	set_current_thread(new_thread);
	thread_switch(temp, current_thread);		// Switching to new_thread
}

/* Scheduler end to yiels to all threads till all threads complete */
void scheduler_end()
{
	while(is_empty(ready_queue)!=1)    // checking queue is empty
	{
		//		printf("Main thread yielding\n");
		yield();       // yield to the next thread in queue
	}
}

/* Function to Set current thread to DONE when thread completes and then yields */
void thread_finish()
{
	current_thread->state=DONE;
	//	printf("%s DONE\n",(char *)current_thread->initial_arg);
	yield();   // yield to the next thread in queue
}

