#ifndef KUDOS_PROC_MUTEX
#define KUDOS_PROC_MUTEX

#include "kernel/spinlock.h"

typedef struct {
  int taken;
  spinlock_t slock;
} mutex_t;

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

#endif /* KUDOS_PROC_MUTEX */
