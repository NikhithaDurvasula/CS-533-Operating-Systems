#include "scheduler.h"
#include <stdio.h>
#include <fcntl.h>
void print_nth_prime(void * pn) {
  int n = *(int *) pn;
  int c = 1, i = 1;
  while(c <= n) {
    ++i;
    int j, isprime = 1;
    for(j = 2; j < i; ++j) {
      if(i % j == 0) {
        isprime = 0;
        break;
      }
    }
    if(isprime) {
      ++c;
    }
    yield();
  }
  printf("\n%dth prime: %d\n", n, i);

}


void asyn_call(void * args) {

  unsigned char *buffer=malloc(sizeof(char *)*200);
  int fd = open ("unknown", O_RDONLY);
  read_wrap (0, buffer,6 );
  printf("buffer value on standard input %s\n",buffer);
  // create buffer variable and allocate memory
  close(fd);
  /* code */
}




int main(void) {
  scheduler_begin();

  int n1 = 20000, n2 = 10000, n3 = 30000;
  thread_fork(print_nth_prime, &n1);
  thread_fork(print_nth_prime, &n2);
  thread_fork(print_nth_prime, &n3);
  thread_fork(asyn_call, (void *)"Thread");

  scheduler_end();
}
