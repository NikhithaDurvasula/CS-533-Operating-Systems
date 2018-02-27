/* This is main Program */
#include <stdio.h>
#include<stdlib.h>

#include <string.h>
#include<fcntl.h>
#include "scheduler.h"
struct mutex mutex_test;// mutex for checking mutex lock functionality
AO_TS_t spinlock_printf = AO_TS_INITIALIZER;	
/* test function to check writing in to global shared variable by different threads */
int shared_counter=0;
void test(void * arg )
{
	mutex_lock(&mutex_test); // lock the mutex
	++shared_counter; 
	yield();	
	mutex_unlock(&mutex_test);		// unlocking mutex

}

void main()
{
	/* Test for condition variable using producer consumer problem, one producer and three consumers*/
	mutex_init(&mutex_test);	// mutex intialization
	scheduler_begin(); // scheduler begin


	// Creating 100 threads to run on same function
	int i=0;
	while(i<100)
	{
		thread_fork(&test,(void*)(char *)"thread");
		i++;
	}
	scheduler_end();					// scheduler end
	spinlock_lock(&spinlock_printf);
	printf("Shared Variable %d\n",shared_counter);
	spinlock_unlock(&spinlock_printf);
}
