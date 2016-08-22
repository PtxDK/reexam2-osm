/*
 * Process startup.
 */

#include <arch.h>

#include <stddef.h>             // NULL, comes from GCC.

#include "proc/process.h"
#include "proc/elf.h"
#include "kernel/thread.h"
#include "kernel/assert.h"
#include "kernel/interrupt.h"
#include "kernel/config.h"
#include "fs/vfs.h"
#include "kernel/sleepq.h"
#include "vm/memory.h"

#include "drivers/device.h"     // device_*
#include "drivers/gcd.h"        // gcd_*
#include "kernel/assert.h"      // KERNEL_ASSERT
#include "proc/syscall.h"       // FILEHANDLE_*

/** @name Process startup
 *
 * This module contains functions for starting and managing userland
 * processes.
 */
extern void process_set_pagetable(pagetable_t*);

process_control_block_t process_table[PROCESS_MAX_PROCESSES];

spinlock_t process_table_slock;

void process_reset(process_id_t pid) {
  process_control_block_t *process = &process_table[pid];

  process->state = PROCESS_FREE;
  process->retval = 0;
  process->n_threads = 0;
  process->free_stack_top = 0;
}

/* Initialize process table and spinlock */
void process_init() {
  int i;
  spinlock_reset(&process_table_slock);
  for (i = 0; i < PROCESS_MAX_PROCESSES; ++i) {
    process_reset(i);
  }
}

/* Find a free slot in the process table. Returns PROCESS_MAX_PROCESSES
 * if the table is full. */
process_id_t alloc_process_id() {
  int i;
  interrupt_status_t intr_status;

  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);
  for (i = 0; i < PROCESS_MAX_PROCESSES; ++i) {
    if (process_table[i].state == PROCESS_FREE) {
      process_table[i].state = PROCESS_RUNNING;
      process_table[i].parent = process_get_current_process();
      break;
    }
  }

  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);
  return i;
}

/* Return non-zero on error. */
int setup_new_process(process_id_t pid, TID_t thread,
                      const char *executable, const char **argv_src,
                      virtaddr_t *entry_point, virtaddr_t *stack_top)
{
  pagetable_t *pagetable;
  elf_info_t elf;
  openfile_t file;
  uintptr_t phys_page;
  int i, res;
  process_control_block_t *process;
  thread_table_t *thread_entry = thread_get_thread_entry(thread);

  process = &process_table[pid];

  int argc = 1;
  virtaddr_t argv_begin;
  virtaddr_t argv_dst;
  int argv_elem_size;
  virtaddr_t argv_elem_dst;

  file = vfs_open((char *)executable);

  /* Make sure the file existed and was a valid ELF file */
  if (file < 0) {
    return -1;
  }

  res = elf_parse_header(&elf, file);
  if (res < 0) {
    return -1;
  }

  /* Trivial and naive sanity check for entry point: */
  if (elf.entry_point < PAGE_SIZE) {
    return -1;
  }

  *entry_point = elf.entry_point;

  pagetable = vm_create_pagetable(thread);

  thread_entry->pagetable = pagetable;

  process->n_threads = 1;

  /* Allocate and map stack */
  process->stack_bot = USERLAND_STACK_TOP & PAGE_SIZE_MASK;
  for(i = 0; i < CONFIG_USERLAND_STACK_SIZE; i++) {
    phys_page = physmem_allocblock();
    KERNEL_ASSERT(phys_page != 0);
    /* Zero the page */
    memoryset((void*)ADDR_PHYS_TO_KERNEL(phys_page), 0, PAGE_SIZE);
    vm_map(pagetable, phys_page,
           process->stack_bot - i*PAGE_SIZE, 1);
  }
  process->stack_bot -= CONFIG_USERLAND_STACK_SIZE * PAGE_SIZE;

  /* Allocate and map pages for the ELF segments. We assume that
     the segments begin at a page boundary. (The linker script
     in the userland directory helps users get this right.) */
  for(i = 0; i < (int)elf.ro_pages; i++) {
    int left_to_read = elf.ro_size - i*PAGE_SIZE;
    phys_page = physmem_allocblock();
    KERNEL_ASSERT(phys_page != 0);
    /* Zero the page */
    memoryset((void*)ADDR_PHYS_TO_KERNEL(phys_page), 0, PAGE_SIZE);
    /* Fill the page from ro segment */
    if (left_to_read > 0) {
      KERNEL_ASSERT(vfs_seek(file, elf.ro_location + i*PAGE_SIZE) == VFS_OK);
      KERNEL_ASSERT(vfs_read(file, (void*)ADDR_PHYS_TO_KERNEL(phys_page),
                             MIN(PAGE_SIZE, left_to_read))
                    == (int) MIN(PAGE_SIZE, left_to_read));
    }
    vm_map(pagetable, phys_page,
           elf.ro_vaddr + i*PAGE_SIZE, 0);
  }

  for(i = 0; i < (int)elf.rw_pages; i++) {
    int left_to_read = elf.rw_size - i*PAGE_SIZE;
    phys_page = physmem_allocblock();
    KERNEL_ASSERT(phys_page != 0);
    /* Zero the page */
    memoryset((void*)ADDR_PHYS_TO_KERNEL(phys_page), 0, PAGE_SIZE);
    /* Fill the page from rw segment */
    if (left_to_read > 0) {
      KERNEL_ASSERT(vfs_seek(file, elf.rw_location + i*PAGE_SIZE) == VFS_OK);
      KERNEL_ASSERT(vfs_read(file, (void*)ADDR_PHYS_TO_KERNEL(phys_page),
                             MIN(PAGE_SIZE, left_to_read))
                    == (int) MIN(PAGE_SIZE, left_to_read));
    }
    vm_map(pagetable, phys_page,
           elf.rw_vaddr + i*PAGE_SIZE, 1);
  }

  /* Done with the file. */
  vfs_close(file);

  /* Set up argc and argv on the stack. */

  /* Start by preparing ancillary information for the new process argv. */
  if (argv_src != NULL)
    for (i = 0; argv_src[i] != NULL; i++) {
      argc++;
    }

  argv_begin = USERLAND_STACK_TOP - (argc * sizeof(virtaddr_t));
  argv_dst = argv_begin;

  /* Prepare for copying executable. */
  argv_elem_size = strlen(executable) + 1;
  argv_elem_dst = argv_dst - wordpad(argv_elem_size);

  /* Copy executable to argv[0] location. */
  vm_memwrite(pagetable,
              argv_elem_size,
              argv_elem_dst,
              executable);
  /* Set argv[i] */
  vm_memwrite(pagetable,
              sizeof(virtaddr_t),
              argv_dst,
              &argv_elem_dst);

  /* Move argv_dst to &argv[1]. */
  argv_dst += sizeof(virtaddr_t);

  if (argv_src != NULL) {
    for (i = 0; argv_src[i] != NULL; i++) {
      /* Compute the size of argv[i+1] */
      argv_elem_size = strlen(argv_src[i]) + 1;
      argv_elem_dst -= wordpad(argv_elem_size);

      /* Write the 'i+1'th element of argv */
      vm_memwrite(pagetable,
                  argv_elem_size,
                  argv_elem_dst,
                  argv_src[i]);

      /* Write argv[i+1] */
      vm_memwrite(pagetable,
                  sizeof(virtaddr_t),
                  argv_dst,
                  &argv_elem_dst);

      /* Move argv_dst to next element of argv. */
      argv_dst += sizeof(virtaddr_t);
    }
  }

  /* Write argc to the stack. */
  vm_memwrite(pagetable,
              sizeof(int),
              argv_elem_dst - sizeof(int),
              &argc);
  /* Write argv to the stack. */
  vm_memwrite(pagetable,
              sizeof(virtaddr_t),
              argv_elem_dst - sizeof(int) - sizeof(virtaddr_t),
              &argv_begin);

  /* Stack pointer points at argv. */
  *stack_top = argv_elem_dst - sizeof(int) - sizeof(virtaddr_t);

  return 0;
}

void process_run(process_id_t pid) {
  context_t user_context;

  thread_table_t *my_thread = thread_get_current_thread_entry();

  /* If my process is a zombie, that means initialisation failed. */
  if (process_table[pid].state == PROCESS_ZOMBIE) {
    if (my_thread->pagetable) {
      vm_destroy_pagetable(my_thread->pagetable);
      my_thread->pagetable = NULL;
    }
    thread_finish();
  }

  process_set_pagetable(my_thread->pagetable);
  my_thread->process_id = pid;
  my_thread->pagetable = my_thread->pagetable;

  /* Initialize the user context. (Status register is handled by
     thread_goto_userland) */
  memoryset(&user_context, 0, sizeof(user_context));

  _context_set_ip(&user_context, process_table[pid].entry_point);
  _context_set_sp(&user_context, process_table[pid].stack_top);

  thread_goto_userland(&user_context);
}

static int tty_read(void *buffer, int length) {
  device_t *dev;
  gcd_t *gcd;

  dev = device_get(TYPECODE_TTY, 0);
  if (dev == NULL) return IO_TTY_UNAVAILABLE;
  gcd = (gcd_t *)dev->generic_device;
  if (gcd == NULL) return IO_TTY_UNAVAILABLE;

  return gcd->read(gcd, buffer, length);
}

static int tty_write(const void *buffer, int length) {
  device_t *dev;
  gcd_t *gcd;

  dev = device_get(TYPECODE_TTY, 0);
  if (dev == NULL) return IO_TTY_UNAVAILABLE;
  gcd = (gcd_t *)dev->generic_device;
  if (gcd == NULL) return IO_TTY_UNAVAILABLE;

  return gcd->write(gcd, buffer, length);
}

int process_read(int filehandle, void *buffer, int length) {
  int retval = 0;

  if (length < 0) {
    retval = IO_NEGATIVE_LENGTH;
  } else if (!IN_USERLAND(buffer) || !IN_USERLAND(buffer + length)) {
    KERNEL_PANIC("SEGFAULT\n");
  } else if (filehandle == FILEHANDLE_STDIN) {
    retval = tty_read(buffer, length);
  } else if (filehandle == FILEHANDLE_STDOUT
            || filehandle == FILEHANDLE_STDERR) {
    retval = IO_INVALID_HANDLE;
  } else {
    retval = IO_NOT_IMPLEMENTED;
  }

  return retval;
}

int process_write(int filehandle, const void *buffer, int length) {
  int retval = 0;

  if (length < 0) {
    retval = IO_NEGATIVE_LENGTH;
  } else if (!IN_USERLAND(buffer) || !IN_USERLAND(buffer + length)) {
    KERNEL_PANIC("SEGFAULT\n");
  } else if (filehandle == FILEHANDLE_STDIN) {
    retval = IO_INVALID_HANDLE;
  } else if (filehandle == FILEHANDLE_STDOUT
            || filehandle == FILEHANDLE_STDERR) {
    retval = tty_write(buffer, length);
  } else {
    retval = IO_NOT_IMPLEMENTED;
  }

  return retval;
}

process_id_t process_spawn(const char *executable, const char **argv) {
  TID_t tid;
  process_id_t pid = alloc_process_id();
  int ret;

  if (pid == PROCESS_MAX_PROCESSES) {
    return PROCESS_PTABLE_FULL;
  }

  tid = thread_create((void (*)(uint32_t))(&process_run), pid);
  ret = setup_new_process(pid, tid, executable, argv,
                          &process_table[pid].entry_point,
                          &process_table[pid].stack_top);

  if (ret != 0) {
    process_table[pid].state = PROCESS_ZOMBIE;
    ret = -1;
  } else {
    ret = pid;
  }

  thread_run(tid);
  return ret;
}

process_id_t process_get_current_process(void) {
  return thread_get_current_thread_entry()->process_id;
}

process_control_block_t *process_get_current_process_entry(void) {
  return &process_table[process_get_current_process()];
}

int process_join(process_id_t pid) {
  int retval;
  interrupt_status_t intr_status;

  /* Only join with legal pids */
  if (pid < 0 || pid >= PROCESS_MAX_PROCESSES ||
      process_table[pid].parent != process_get_current_process())
    return PROCESS_ILLEGAL_JOIN;

  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  /* The thread could be zombie even though it wakes us (maybe). */
  while (process_table[pid].state != PROCESS_ZOMBIE) {
    sleepq_add(&process_table[pid].retval);
    spinlock_release(&process_table_slock);
    thread_switch();
    spinlock_acquire(&process_table_slock);
  }

  retval = process_table[pid].retval;
  process_reset(pid);

  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);
  return retval;
}

/**
 * This function inserts the userspace thread stack in a list of free
 * stacks maintained in the process table entry.  This means that
 * when/if the next thread is created, we can reuse one of the old
 * stacks, and reduce memory usage.  Note that the stack is not really
 * "deallocated" per se, and still counts towards the 64KiB memory
 * limit for processes.  This is a simple mechanism, not a very good
 * one.  This function assumes that the process table is already
 * locked.
 * 
 * @param my_thread The thread whose stack should be deallocated.
 *
 */
void process_free_stack(
    process_control_block_t *process,
    thread_table_t *thread) {

  /* Assume we have lock on the process table. */

  uint32_t old_free_list = process->free_stack_top;

  /* Find the stack by applying a mask to the stack pointer. */
  uint32_t stack =
      (thread->user_context->cpu_regs[MIPS_REGISTER_SP]
       & USERLAND_STACK_MASK)
      + CONFIG_USERLAND_STACK_SIZE*PAGE_SIZE
      - 4;

  KERNEL_ASSERT(stack >= process->stack_bot);

  process->free_stack_top = stack;
  *(uint32_t*)stack = old_free_list;
}

void process_exit(int retval) {
  interrupt_status_t intr_status;
  process_control_block_t *process;
  thread_table_t *thread;

  process = process_get_current_process_entry();
  thread = thread_get_current_thread_entry();

  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  // Always reset the thread pagetable (to make thread_finish() work) and
  // decrement the number of threads, but only actually kill off the process if
  // this is the last thread in the process.

  process->n_threads--;
  process_free_stack(process, thread);

  if (process->n_threads == 0) {
    vm_destroy_pagetable(thread->pagetable);

    process->state = PROCESS_ZOMBIE;
    process->retval = retval;
    sleepq_wake_all(&process->retval);
  }

  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);

  thread->pagetable = NULL;
  thread_finish();
}

/**
 * We need to pass a bunch of data to the new thread, but we can only
 * pass a single 32 bit number!  How do we deal with that?  Simple -
 * we allocate a structure on the stack of the forking kernel thread
 * containing all the data we need, with a 'done' field that indicates
 * when the new thread has copied over the data.  See process_fork().
 */
typedef struct thread_params_t {
  volatile uint32_t done; /* Don't cache in register. */
  void (*func)(int);
  int arg;
  process_id_t pid;
  pagetable_t *pagetable;
  virtaddr_t stack_top;
} thread_params_t;

void process_thread_run(thread_params_t *params) {
  context_t user_context;
  uint32_t phys_page;
  int i;
  interrupt_status_t intr_status;
  thread_table_t *thread;
  virtaddr_t stack_top;

  thread = thread_get_current_thread_entry();

  /* Copy thread parameters. */
  int arg = params->arg;
  void (*func)(int) = params->func;
  process_id_t pid = thread->process_id = params->pid;
  thread->pagetable = params->pagetable;
  params->done = 1; /* OK, we don't need params any more. */

  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  /* Allocate thread stack */
  if (process_table[pid].free_stack_top != 0) {
    /* Reuse old thread stack. */
    stack_top = process_table[pid].free_stack_top;
    process_table[pid].free_stack_top =
           *(uint32_t*)process_table[pid].free_stack_top;
  } else {
    /* Allocate physical pages (frames) for the stack. */
    for (i = 0; i < CONFIG_USERLAND_STACK_SIZE; i++) {
      phys_page = physmem_allocblock();
      KERNEL_ASSERT(phys_page != 0);
      vm_map(thread->pagetable, phys_page,
             process_table[pid].stack_bot - (i+1)*PAGE_SIZE, 1);
    }
    stack_top = process_table[pid].stack_bot - 4;
    process_table[pid].stack_bot -= CONFIG_USERLAND_STACK_SIZE * PAGE_SIZE;
  }

  /* Set up userspace environment. */
  memoryset(&user_context, 0, sizeof(user_context));
  _context_init(&user_context,
                (virtaddr_t) func,
                0, // Someone should put something sensible here.
                stack_top,
                (uint32_t) arg);

  tlb_fill(thread->pagetable);

  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);

  thread_goto_userland(&user_context);
}

int process_thread(void (*func)(int), int arg) {
  TID_t tid;
  thread_table_t *thread = thread_get_current_thread_entry();
  process_id_t pid = thread->process_id;
  interrupt_status_t intr_status;
  thread_params_t params;
  params.done = 0;
  params.func = func;
  params.arg = arg;
  params.pid = pid;
  params.pagetable = thread->pagetable;

  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  tid = thread_create((void (*)(uint32_t))(process_thread_run), (uint32_t)&params);
  if (tid < 0) {
    spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);
    return PROCESS_CANNOT_THREAD;
  }

  if (process_table[pid].n_threads == 16) {
     spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);
    return PROCESS_TOO_MANY_THREADS;
  }

  process_table[pid].n_threads++;

  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);

  thread_run(tid);

  /* params will be dellocated when we return, so don't until the
     new thread is ready. */
  while (!params.done);

  return tid;
}

int process_gettid() {
  return thread_get_current_thread();
}
