#pragma once

#include <stdbool.h>

#include "../include/thread.h"

/// TID of the idle thread.
extern thread_tid_t THREADS_IDLE_TID;

/// TID of currently active thread.
///
/// Set to new value right before switch.
extern thread_tid_t THREADS_ACTIVE_TID;

/// Number of threads we currently support
#define THREADS_NUM 16

/// Stack space allocated to each thread.
/// Will be dynamic in future once proper memory management is implemented.
#define THREAD_STACK_SIZE 4096

/// Data structure that owns everything related to a particular thread, for now.
struct thread_t {
    /// Whether the data structure is occupied or may be reused for a new thread.
    bool occupied;
    /// TCB of this thread.
    struct thread_tcb_t tcb;
    /// Memory for the thread's stack.
    unsigned char stack[THREAD_STACK_SIZE];
};

/// A statically-allocated pool of threads.
///
/// Only functionality internal to the threading subsystem should ever access this.
extern struct thread_t THREADS[THREADS_NUM];

/// Look up a thread based on it's TID.
///
/// Return NULL if the TID does not belong to a currently alive thread.
///
/// Reference lives at least until the thread terminates.
struct thread_t *lookup_thread_by_tid(thread_tid_t tid);

/// Look up a thread's index in the thread array based on it's TID.
///
/// Return -1 if the TID does not belong to a currently alive thread.
///
/// Primarily of use for scheduling algorithms.
int lookup_idx_by_tid(thread_tid_t tid, size_t *idx);

/// Dump the given CPU state struct via kprintf.
void print_cpu_state(const struct thread_cpu_state_t *const t_cpu);
