#include<stdio.h>
#include<stdlib.h>
#define SIZE (1024*1024)
struct thread{
unsigned char* stack_pointer;
void (*initial_function)(void*);
void* initial_argument;
}st1;

struct thread *current_thread = &st1;
struct thread *inactive_thread = &st1;
void thread_start(struct thread * old, struct thread * new);
void thread_switch(struct thread * old, struct thread * new);

int summorial(int n) {
    return n == 0 ? 1 : n + summorial(n-1);
  }

void fun_with_threads(void * arg) {
    int n = *(int*) arg;
    printf("%d summorial = %d\n", n, summorial(n));
  }

void yield() {
    struct thread * temp = current_thread;
    current_thread = inactive_thread;
    inactive_thread = temp;
    thread_switch(inactive_thread, current_thread);
  }

void thread_wrap() {
    current_thread->initial_function(current_thread->initial_argument);
    yield();
  }

void main()
{
current_thread=(struct thread *)malloc(sizeof(struct thread));
inactive_thread=(struct thread *)malloc(sizeof(struct thread));
current_thread->initial_function = fun_with_threads;
int *p = malloc(sizeof(int));
*p = 5;
current_thread->initial_argument = p;
current_thread->stack_pointer = malloc(SIZE) + (SIZE);
current_thread->initial_function(current_thread->initial_argument);
thread_start(inactive_thread, current_thread);
}
~
