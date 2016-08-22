#include "proc/cond.h"

#include "kernel/interrupt.h"
#include "kernel/thread.h"
#include "kernel/sleepq.h"

/* Wait for the condition to occur. There is no guarantee that the condition
 * "holds" when the function returns.
 * Pre-condition: The process must hold the lock prior to calling wait. */
void cond_wait(cond_t *cond, mutex_t *mutex) {
  interrupt_status_t intr_status;

  intr_status = _interrupt_disable();

  sleepq_add(cond);
  mutex_unlock(mutex);
  thread_switch();
  mutex_lock(mutex);

  _interrupt_set_state(intr_status);
}

/* Signal one waiting process, if any, that the condition has occured. */
void cond_signal(cond_t *cond) {
  sleepq_wake(cond);
}

/* Signal all waiting processes, if any, that the condition has occured. */
void cond_broadcast(cond_t *cond) {
  sleepq_wake_all(cond);
}
