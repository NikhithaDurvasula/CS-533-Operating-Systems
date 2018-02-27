/* This is main Program */
#include <stdio.h>
#include<stdlib.h>
#include "scheduler.h"
#include <string.h>
#include<fcntl.h>
long fib[45]; // Instalizing first two values in fibonacci series
int counter=2; // Intializing counter to start next series

void is_prime(void * arg )
{
	long number = (long)arg;
	int count =0, counter=1;
	for(;counter<=number/2;counter++)
	{
		if(number%counter==0) // checking reminder with each number till half of the given number
		{		
			count++;        
			yield();
		}  // increment count 
		yield();    // yield to the next thread in queue
		if(count>1)  // break if  divided by more than one number
			break;
	}
	if(count==1) // checking number only divided perfectly only once
	{
		printf(" \n%lu is prime\n", number);
	}
	else
		printf(" \n%lu is not prime\n", number);
}
/* fibonacci series evaluating function*/
void fibonacci(void * arg )
{ 
	void ** args = (void**)arg;
	fib[0] = (long)args[0];			// intializing first two values in fibonacci series
	fib[1] = (long)args[1];
	while( counter < 45)
	{		
		fib[counter] = fib[counter-1] + fib[counter-2]; // getting next value in series
		counter++;
		yield();					// yield to the next thread in queue
	}
	printf("\nfibonacci series calculated\n");
}




/* Main function- program starting point */
void main()
{
	scheduler_begin(); // scheduler begin
	int loop_counter;		// counter variable for fibonacci series\ create new thread for async_call
	thread_fork(&is_prime,(void*)9791);		// Thread_fork to check prime number for number 3
	thread_fork(&is_prime,(void*)3);	// Thread_fork to check prime number for number 9791
	void * args[] = {(void*)0, (void*)1};			// intializing fibonacci series first two values
	thread_fork(&fibonacci,args);				//  thred fork to run fibonacci series

	scheduler_end();					// scheduler end

	/* printing fibonacci series */
	printf("\nFibonacci series with thread\n");  		
	for(loop_counter=0;loop_counter<43;loop_counter++)
	{
		printf("%lu\t",fib[loop_counter]);
	}
}
