#include "rwlock.h"

int main() {
  rwlock_t rwlock = rwlock_create();

  printf("rwlock_rlock_acquire returned %d.\n",
    rwlock_rlock_acquire(rwlock));

  syscall_halt();
}
