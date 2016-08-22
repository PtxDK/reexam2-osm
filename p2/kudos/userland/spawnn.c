#include "lib.h"

#define tostart "[disk]tenlines.mips32"
#define HOWMANY 10

int main(void) {
  int pids[HOWMANY];

  for (int i = 0; i < HOWMANY; i++) {
    if ((pids[i] = syscall_spawn(tostart, NULL)) < 0) {
      printf("spawnn: process_spawn() numer %d failed\n", i);
      syscall_halt();
    }
  }

  for (int i = 0; i < HOWMANY; i++) {
    syscall_join(pids[i]);
  }

  printf("spawnn: Joined every child\n");
  syscall_halt();
}
