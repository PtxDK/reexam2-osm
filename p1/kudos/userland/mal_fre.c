#include "lib.h"

#define N_THREADS (10)

mutex_t mutex;
cond_t cond;

void foo(void* a) {
  a=a;
  printf("FOOOOO\n");
  int* allocated_space;
  allocated_space = malloc(32);
  printf("%d", allocated_space);
  printf("BAAAAAR\n");
  free(allocated_space);
  syscall_exit(0);
}

int main() {


  printf("Attempting to allocate 32 bytes 5 times..\n");

  heap_init();

  syscall_thread( &foo, NULL);
  syscall_thread( &foo, NULL);
  syscall_thread( &foo, NULL);
  syscall_thread( &foo, NULL);
  syscall_thread( &foo, NULL);

/*
  void *allocated_space;

  allocated_space = malloc(space_to_allocate);


  printf("%d", allocated_space)
  printf("Space Allocated\n");

  free(allocated_space);

  printf("Space Freed again\n");
*/

/*
  printf("Created the following threads: { ");
  syscall_halt();
  for (i = 0; i < N_THREADS; ++i) {
  }

  syscall_cond_broadcast(&cond);
*/
  return 0;
}

