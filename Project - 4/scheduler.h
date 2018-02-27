typedef enum {
     RUNNING, // The thread is currently running.
     READY,   // The thread is not running, but is runnable.
     BLOCKED, // The thread is not running, and not runnable.
     DONE     // The thread has finished.
   } state_t;


struct thread{
unsigned char* stack_pointer;
void (*initial_function)(void*);
void* initial_argument;
state_t state;
struct mutex *mutexvar;
struct condition *conditionvar;
}st1;

void scheduler_begin();
 struct thread* thread_fork(void(*target)(void*), void * arg);
 void yield();
 void scheduler_end();

 extern struct thread * current_thread;


struct mutex
{
       int held;
//      struct thread *locked_thread;
       struct queue *waiting_threads;
};


struct condition
{
    struct queue *waiting_threads;
};



void thread_join(struct thread*);
void condition_signal(struct condition *conditionvar);
void condition_broadcast(struct condition *conditionvar);
void condition_init(struct condition *conditionvar);
void condition_wait(struct condition *conditionvar, struct mutex *mutexvar);
void condition_signal(struct condition *conditionvar);
void mutex_init(struct mutex *mutexvar);
void mutex_lock(struct mutex *mutexvar);
void mutex_unlock(struct mutex *mutexvar);
