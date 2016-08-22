#include "lib.h"

int main() {
  int pid;

  pid = syscall_spawn("[disk]threads.mips32", NULL);
  syscall_join(pid);

  syscall_halt();
}
