#include "lib.h"

#define N_THREADS (10)

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

  return 0;
}

