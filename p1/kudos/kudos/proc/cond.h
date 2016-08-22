#ifndef KUDOS_PROC_COND
#define KUDOS_PROC_COND

#include "lib/types.h"
#include "proc/mutex.h"

typedef struct {
  uint32_t unused; // We only have the field to make conditions adressable.
} cond_t;

void cond_wait(cond_t *cond, mutex_t *mutex);
void cond_signal(cond_t *cond);
void cond_broadcast(cond_t *cond);

#endif /* KUDOS_PROC_COND */
