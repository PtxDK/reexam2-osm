/*
 * System calls.
 */

#ifndef KUDOS_PROC_SYSCALL
#define KUDOS_PROC_SYSCALL

/* Syscall function numbers. You may add to this list but do not
 * modify the existing ones.
 */
#define SYSCALL_HALT      0x001

#define SYSCALL_SPAWN     0x101
#define SYSCALL_EXIT      0x102
#define SYSCALL_JOIN      0x103
#define SYSCALL_FORK      0x104
#define SYSCALL_MEMLIMIT  0x105
#define SYSCALL_THREAD    0x106
#define SYSCALL_GETTID    0x107

#define SYSCALL_OPEN      0x201
#define SYSCALL_CLOSE     0x202
#define SYSCALL_SEEK      0x203
#define SYSCALL_READ      0x204
#define SYSCALL_WRITE     0x205
#define SYSCALL_CREATE    0x206
#define SYSCALL_DELETE    0x207
#define SYSCALL_FILECOUNT 0x208
#define SYSCALL_FILE      0x209

#define SYSCALL_MUTEX_INIT    0x301
#define SYSCALL_MUTEX_LOCK    0x302
#define SYSCALL_MUTEX_UNLOCK  0x303

#define SYSCALL_COND_WAIT       0x304
#define SYSCALL_COND_SIGNAL     0x305
#define SYSCALL_COND_BROADCAST  0x306

/* When userland program reads or writes these already open files it
 * actually accesses the console.
 */
#define FILEHANDLE_STDIN  0
#define FILEHANDLE_STDOUT 1
#define FILEHANDLE_STDERR 2

#define IO_INVALID_HANDLE  (-1)
#define IO_NOT_IMPLEMENTED (-2)
#define IO_NEGATIVE_LENGTH (-3)
#define IO_TTY_UNAVAILABLE (-4)

#endif
