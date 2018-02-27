
/* scheduler API Program scheduler.c */
#include<stdio.h>
#include<stdlib.h>
#include "queue.h"   // including queue header file
#include "scheduler.h"	// including scheduler header

struct queue *ready_queue;

/* condition variable intialization function */
void condition_init(struct condition *conditionvar)
{
	conditionvar->waiting_con_queue=malloc(sizeof(struct queue *)); // allocating memory for queue conditionvar->waiting_con_queue
	conditionvar->waiting_con_queue->head=NULL;   			// intializing head to NULL
}
/*  function to keep a thread in waiting queue based on given condition */
void condition_wait(struct condition *conditionvar)
{	
	current_thread->state=BLOCKED;		//Changing status of current thread to blocked
	thread_enqueue((conditionvar->waiting_con_queue),current_thread);// enqueing in to the current thread to conditionvar waiting queue
	yield();			// yielding in to the next thread
}
/* Function to signal a waiting thread on some condition */
void condition_signal(struct condition *conditionvar)
{
	if(is_empty(conditionvar->waiting_con_queue)!=1) /* checking whether the queue is empty*/
	{
		struct thread_t *new_thread=thread_dequeue(conditionvar->waiting_con_queue); // dequeuing thread from waiting queue
		new_thread->state=READY;	// changing state to ready
		thread_enqueue(ready_queue,new_thread);	// enque in to the ready queue
	}
}
/* function to wakeup all threads waiting in queue for a given condition */
void condition_broadcast(struct condition *conditionvar)
{
	while(is_empty(conditionvar->waiting_con_queue)!=1)	// loop till the queue is empty
	{
		struct thread_t *new_thread=thread_dequeue(conditionvar->waiting_con_queue);// dequeuing a thread from waiting queue
		new_thread->state=READY;		 // changing state to ready
		thread_enqueue(ready_queue,new_thread);	//enqueue in to the ready queue
	} 
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

/* Scheduler begin function to create ready queue */ 
void scheduler_begin()
{
	ready_queue=malloc(sizeof(struct queue *)); // allocating ready_queue memory
	ready_queue->head=NULL; 				// intializing head to NULL
	set_current_thread(malloc(sizeof(struct thread_t))); 	// Allocating memory for current_thread
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
	yield();   // yield to the next thread in queue
}
