/*
 * System calls.
 */
#include <cswitch.h>
#include "proc/syscall.h"
#include "kernel/halt.h"
#include "kernel/panic.h"
#include "lib/libc.h"
#include "kernel/assert.h"
#include "vm/memory.h"
#include "proc/process.h"
#include "proc/mutex.h"
#include "proc/cond.h"      // cond_*

/**
 * Handle system calls. Interrupts are enabled when this function is
 * called.
 */
uintptr_t syscall_entry(uintptr_t syscall,
                        uintptr_t arg0, uintptr_t arg1, uintptr_t arg2)
{
  int retval = 0;

  arg0 = arg0;
  arg1 = arg1;
  arg2 = arg2;
  /* When a syscall is executed in userland, register a0 contains
   * the number of the syscall. Registers a1, a2 and a3 contain the
   * arguments of the syscall. The userland code expects that after
   * returning from the syscall instruction the return value of the
   * syscall is found in register v0. Before entering this function
   * the userland context has been saved to user_context and after
   * returning from this function the userland context will be
   * restored from user_context.
   */
  switch(syscall) {
  case SYSCALL_HALT:
    halt_kernel();
    break;
  case SYSCALL_READ:
    retval = process_read(arg0, (void*)arg1, arg2);
    break;
  case SYSCALL_WRITE:
    retval = process_write(arg0, (const void*)arg1, arg2);
    break;
  case SYSCALL_SPAWN:
    return process_spawn((char*) arg0, (char const**) arg1);
    break;
  case SYSCALL_EXIT:
    process_exit((process_id_t) arg0);
    break;
  case SYSCALL_JOIN:
    retval = process_join((process_id_t) arg0);
    break;
  case SYSCALL_THREAD:
    retval = process_thread((void (*)(int)) arg0, arg1);
    break;
  case SYSCALL_GETTID:
    retval = process_gettid();
    break;
  case SYSCALL_MUTEX_INIT:
    mutex_init((mutex_t*)arg0);
    break;
  case SYSCALL_MUTEX_LOCK:
    mutex_lock((mutex_t*)arg0);
    break;
  case SYSCALL_MUTEX_UNLOCK:
    mutex_unlock((mutex_t*)arg0);
    break;
  case SYSCALL_COND_WAIT:
    cond_wait((cond_t*)arg0, (mutex_t*)arg1);
    break;
  case SYSCALL_COND_SIGNAL:
    cond_signal((cond_t*)arg0);
    break;
  case SYSCALL_COND_BROADCAST:
    cond_broadcast((cond_t*)arg0);
    break;
  default:
    KERNEL_PANIC("Unhandled system call\n");
  }

  return retval;
}
