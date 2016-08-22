#ifndef RWLOCK_H
#define RWLOCK_H

#include "lib.h"

typedef int rwlock_t; // FIXME: You probably want something other than int.

rwlock_t rwlock_create();

int rwlock_rlock_acquire(rwlock_t rwlock);

int rwlock_wlock_acquire(rwlock_t rwlock);

int rwlock_rlock_release(rwlock_t rwlock);

int rwlock_wlock_release(rwlock_t rwlock);

int rwlock_destroy(rwlock_t rwlock);

#endif /* RWLOCK_H */
