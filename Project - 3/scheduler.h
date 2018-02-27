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
}st1;

void scheduler_begin();
  void thread_fork(void(*target)(void*), void * arg);
  void yield();
  void scheduler_end();

  extern struct thread * current_thread;


