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
    yield();
  }

void scheduler_begin(){
current_thread = malloc(sizeof(struct thread));
  //current_thread = &st1;
  current_thread->state = RUNNING;
  ready_list.head = NULL;
  ready_list.tail = NULL;
}

void thread_fork(void(*target)(void*), void * arg){

struct thread * new_thread ;
new_thread = malloc(sizeof(struct thread));
new_thread->stack_pointer = malloc(SIZE) + (SIZE);
 
 new_thread->initial_function = target;
 new_thread->initial_argument = arg;
 
 current_thread->state = READY ;
 thread_enqueue(&ready_list,current_thread);
 
 new_thread->state = RUNNING ;
 struct thread *temp = current_thread ;
 current_thread = new_thread ;
 new_thread = temp ;

 thread_start (new_thread,current_thread);

}

void yield() {
  if(current_thread->state != DONE)
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

while(is_empty(&ready_list)==0)
{
yield();

}
}

