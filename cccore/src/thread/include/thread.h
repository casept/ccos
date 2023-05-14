#pragma once

#include <stddef.h>

#include "../src/tcb.h"
#include "interrupt.h"

/// Prepare a switch to the given thread, by storing old thread's state and loading new thread's registers onto the
/// stack, where the ISR will load them back into the CPU.
void thread_switch_prepare(struct thread_cpu_state_t old_thread_cpu_state, thread_tid_t old_thread_tid,
                           thread_tid_t new_thread_tid, struct interrupt_isr_data_t *isr_data);

/// Kick the entire thread machinery into gear by switching to the idle thread and leaving the kernel's main function.
///
/// You should only call this once, and only outside an interrupt handler.
void __attribute__((noreturn)) thread_go(void);

/// Signature that all thread entrypoint functions must obey.
typedef void (*thread_entrypoint_t)(void);

/// Initialize threading.
///
/// Only call this once.
void thread_threading_init(void);

/// Create a new thread.
///
/// Return `-1` if thread could not be created due to lack of resources.
///
/// Return `0` on success.
///
/// The new thread's TID is written into the outparam.
int thread_create(thread_entrypoint_t entry, thread_tid_t *tid);

/// Destroy the thread and reclaim it's resources.
///
/// Return `-1` if a thread with given ID does not exist.
///
/// Return `0` on success.
int thread_destroy(thread_tid_t tid);

/// Dump the thread state with kprintf.
///
/// Panic when thread does not exist.
void thread_dump_state(thread_tid_t tid);

/// Get TID of the currently active thread.
thread_tid_t thread_get_current_tid(void);
