#include "lib.h"

int main() {
  for (int i = 0; i < 100; i++) {
    printf("Hello number %d!\n", i);
  }
  syscall_exit(1);
}
