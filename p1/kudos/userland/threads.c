#include "lib.h"

#define N_THREADS (10)

mutex_t mutex;
cond_t cond;

void thread(void *arg) {

  arg = arg;

  syscall_cond_wait(&cond, &mutex);
  printf("I am thread %d.\n", syscall_gettid());
  syscall_mutex_unlock(&mutex);

  // Should not be necessary, but just in case not everyone was waiting when
  // the ancestor thread called broadcast.
  syscall_cond_signal(&cond);

  syscall_exit(0);
}

int main() {
  size_t i;
  int threads[N_THREADS];

  syscall_mutex_init(&mutex);

  for (i = 0; i < N_THREADS; ++i) {
    threads[i] = syscall_thread(&thread, (void*) i);
    if (threads[i] < 0) {
      printf("!!! Too many threads. !!!\n");
      syscall_halt();
    }
  }

  printf("Created the following threads: { ");
  for (i = 0; i < N_THREADS; ++i) {
    printf("%d ", threads[i]);
  }
  printf("}.\n");

  syscall_cond_broadcast(&cond);

  return 0;
}
