#include <stdio.h>
#include "scheduler.h"
#include "queue.h"
#define SIZE (1024*1024)
#include <stdlib.h>

struct thread * current_thread;
struct queue ready_list;

void thread_wrap() {
    current_thread->initial_function(current_thread->initial_argument);
    current_thread->state = DONE ;
    mutex_lock(current_thread->mutexvar);
    condition_signal(current_thread->conditionvar);
    mutex_unlock(current_thread->mutexvar);
    yield();
  }

void scheduler_begin(){
current_thread = malloc(sizeof(struct thread));
  //current_thread = &st1;
  current_thread->state = RUNNING;
  ready_list.head = NULL;
  ready_list.tail = NULL;
}

struct thread* thread_fork(void(*target)(void*), void * arg){

struct thread * new_thread ;
new_thread = malloc(sizeof(struct thread));
new_thread->stack_pointer = malloc(SIZE) + (SIZE);
new_thread->initial_function = target;
new_thread->initial_argument = arg;

new_thread->mutexvar = malloc(sizeof(struct mutex));
new_thread->conditionvar = malloc(sizeof(struct condition));
mutex_init(new_thread->mutexvar);
condition_init(new_thread->conditionvar);

current_thread->state = READY ;
thread_enqueue(&ready_list,current_thread);

new_thread->state = RUNNING ;
struct thread *ret_thread = new_thread;
struct thread *temp = current_thread ;
current_thread = new_thread ;
new_thread = temp ;
thread_start (new_thread,current_thread);

return ret_thread;
}


void thread_join(struct thread *t)
{
mutex_lock(t->mutexvar);
while(t->state != DONE)
  {
    condition_wait(t->conditionvar, t->mutexvar);
  }
mutex_unlock(t->mutexvar);
}




void mutex_init(struct mutex *mutexvar)
{
        mutexvar->waiting_threads=malloc(sizeof(struct queue* ));
        mutexvar->waiting_threads->head=malloc(sizeof(struct queue_node));
        mutexvar->waiting_threads->tail=malloc(sizeof(struct queue_node));
        mutexvar->waiting_threads->head=NULL;
        mutexvar->waiting_threads->tail=NULL;
        mutexvar->held = 0;
}



void mutex_lock(struct mutex *mutexvar)
{
        if(mutexvar->held==0)
        {
                mutexvar->held=1;
        }

        else
        {
                current_thread->state=BLOCKED;
                thread_enqueue((mutexvar->waiting_threads),current_thread);
                yield();
        }
}





void mutex_unlock(struct mutex *mutexvar)
{
        if(is_empty(mutexvar->waiting_threads))
        mutexvar->held=0;
        else
        {
                struct thread *new_thread = thread_dequeue(mutexvar->waiting_threads);
                new_thread->state=READY;
                thread_enqueue(&ready_list,new_thread);

        }

}


/* condition variable intialization function */
void condition_init(struct condition *conditionvar)
{
        conditionvar->waiting_threads=malloc(sizeof(struct queue*));
       conditionvar->waiting_threads->head=NULL;
        conditionvar->waiting_threads->tail=NULL;
}


/*  function to keep a thread in waiting queue based on given condition */
void condition_wait(struct condition *conditionvar, struct mutex *mutexvar)
{
        mutex_unlock(mutexvar);
        current_thread->state=BLOCKED;
        thread_enqueue(conditionvar->waiting_threads,current_thread);
        yield();
}



/* Function to signal a waiting thread on some condition */
void condition_signal(struct condition *conditionvar)
{
        if(!is_empty(conditionvar->waiting_threads))
        {
                struct thread *new_thread=thread_dequeue(conditionvar->waiting_threads);
                new_thread->state=READY;
                thread_enqueue(&ready_list,new_thread);
        }
}



/* function to wakeup all threads waiting in queue for a given condition */
void condition_broadcast(struct condition *conditionvar)
{

        while(!is_empty(conditionvar->waiting_threads))
        {
                struct thread *new_thread=thread_dequeue(conditionvar->waiting_threads);
                new_thread->state=READY;
                thread_enqueue(&ready_list,new_thread); 
        }
}


void yield() {
  if(current_thread->state != DONE && current_thread->state != BLOCKED)
  {
  current_thread->state = READY ;
  thread_enqueue(&ready_list,current_thread);
  }
  struct thread *next_thread = thread_dequeue(&ready_list) ;

    next_thread->state = RUNNING ;

    struct thread * temp = current_thread;
    current_thread = next_thread;
    next_thread = temp;

    thread_switch(next_thread, current_thread);
  }

void scheduler_end(){

while(!is_empty(&ready_list))
{
yield();

}
}
