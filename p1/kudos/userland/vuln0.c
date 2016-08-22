#include "lib.h"

// Courtesy of br0ns, modified by oleks.

int main(void) {
  printf("Shooting your kernel.. %d.\n",
    syscall_read(0, (void*) 0x80019d27, 8));
  syscall_halt();
  return 0;
}
