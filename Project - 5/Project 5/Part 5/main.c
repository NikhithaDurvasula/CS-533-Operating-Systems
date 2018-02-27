/* This is main Program */
#include <stdio.h>
#include<stdlib.h>
#define MAX 6		// maximum a producer can produce
#include <string.h>
#include <time.h>
#include<fcntl.h>
#include "scheduler.h"
struct mutex mutex_test;// mutex for checking mutex lock functionality
AO_TS_t spinlock_printf = AO_TS_INITIALIZER;	
/* test function to check writing in to global shared variable by different threads */
long fib[1002]; // Instalizing first two values in fibonacci series
int counter=2; // Intializing counter to start next series
int BufferIndex = 0;	// bufferindex to check all produced and consumed
int i=0;	// variable to maintain all produced
struct condition condc, condp;  // condition variables for consumer and prodicer


void fibonacci(void * arg )
{ 
	mutex_lock(&mutex_test); // lock the mutex
	void ** args = (void**)arg;
	fib[0] = (long)args[0];			// intializing first two values in fibonacci series
	fib[1] = (long)args[1];
	while( counter < 1000)
	{		
		fib[counter] = fib[counter-1] + fib[counter-2]; // getting next value in series
		counter++;					// yield to the next thread in queue
	}
	mutex_unlock(&mutex_test); // lock the mutex
}
/* producer function */
void producer(void * arg)
{
	for (; i < MAX; i++)
	{
		mutex_lock(&mutex_test);	/* protect buffer with lock */
		while (BufferIndex != 0)		      /* If there is something in the buffer then wait */
		{
			mutex_unlock(&mutex_test);
			condition_wait(&condp);  
			mutex_lock(&mutex_test);
		}
		BufferIndex++;
		spinlock_lock(&spinlock_printf);
		printf("producer produce \n");
		spinlock_unlock(&spinlock_printf);

		mutex_unlock(&mutex_test);	/* release the buffer with mutex unlock*/;
		condition_signal(&condc);/* wakeup all consumers on condition broadcast */
	}
}
/* consumer function */
void consumer(void * arg)
{
	while(i <= MAX)	/* checking all consumed */
	{
		mutex_lock(&mutex_test);        /* protect buffer with lock */
		while (BufferIndex == 0)	                  /* once consumes two produced from buffer then  wait */
		{
			mutex_unlock(&mutex_test);
			condition_wait(&condc);
			mutex_lock(&mutex_test);		
		}
		BufferIndex--;
		spinlock_lock(&spinlock_printf);
		printf("consumer consume \n"); 
		spinlock_unlock(&spinlock_printf); 
		mutex_unlock(&mutex_test);      /* release the buffer */
		condition_signal(&condp);	/* wake up producer by signalling producer*/
	}
}

void main()
{
	//time_t timer;
	/* Test for condition variable using producer consumer problem, one producer and three consumers*/
	mutex_init(&mutex_test);	// mutex initialization
	condition_init(&condc);			// condition variable for consumers initialization
	condition_init(&condp);  // condition variable for producers initialization
	scheduler_begin(20); // scheduler begin





	void * args[] = {(void*)0, (void*)1};			// intializing fibonacci series first two values
	int i=0;
	for(i=0;i<100;i++)
	{
		thread_fork(&fibonacci,args);	
	}			//  thred fork to run fibonacci series

	thread_fork(&producer,(void*)"temp");		
	thread_fork(&consumer,(void*)"temp");
	scheduler_end();					// scheduler end

	/*	spinlock_lock(&spinlock_printf);
		printf("\nFibonacci series\n"); 
		int loop_counter;		// counter variable for fibonacci series\ create new thread for async_call 		
		for(loop_counter=0;loop_counter<100;loop_counter++)
		{
		printf("%lu\t",fib[loop_counter]);
		}
		spinlock_unlock(&spinlock_printf); */

}
