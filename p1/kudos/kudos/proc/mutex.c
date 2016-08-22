#include "proc/mutex.h"

#include "kernel/interrupt.h"
#include "kernel/thread.h"
#include "kernel/sleepq.h"

void mutex_init(mutex_t *mutex) {
  mutex->taken = 0;
  spinlock_reset(&mutex->slock);
}

void mutex_lock(mutex_t *mutex) {
  interrupt_status_t intr_status;

  intr_status = _interrupt_disable();
  spinlock_acquire(&mutex->slock);

  while (mutex->taken != 0) {
    sleepq_add(&mutex->taken);
    spinlock_release(&mutex->slock);
    thread_switch();
    spinlock_acquire(&mutex->slock);
  }

  mutex->taken = 1;

  spinlock_release(&mutex->slock);
  _interrupt_set_state(intr_status);
}

void mutex_unlock(mutex_t *mutex) {

  interrupt_status_t intr_status;

  intr_status = _interrupt_disable();
  spinlock_acquire(&mutex->slock);

  mutex->taken = 0;

  spinlock_release(&mutex->slock);
  _interrupt_set_state(intr_status);

  sleepq_wake(&mutex->taken);
}
