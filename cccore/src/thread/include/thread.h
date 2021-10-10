#pragma once

#include "../src/tcb.h"

typedef uint64_t thread_register_t;

/// Data structure holding the thread's CPU state.
///
/// NOTE: This is used by ASM code!
/// Never change anything about this struct without also adjusting that code.
struct __attribute__((packed)) thread_cpu_state_t {
    thread_register_t r15;
    thread_register_t r14;
    thread_register_t r13;
    thread_register_t r12;
    thread_register_t r11;
    thread_register_t r10;
    thread_register_t r9;
    thread_register_t r8;

    thread_register_t rbp;
    thread_register_t rsp;
    thread_register_t rdi;
    thread_register_t rsi;
    thread_register_t rdx;
    thread_register_t rcx;
    thread_register_t rbx;
    thread_register_t rax;

    thread_register_t
        rip;  // Must be down here because it's the last thing popped on thread switch via retq instruction.
};

/// Switch to the given thread.
///
/// Returns `-1` if the thread does not exist.
///
/// Does not return on success, because the execution changes from kernel to the thread.
int thread_switch(struct thread_cpu_state_t old_thread_cpu_state, thread_tid_t old_thread_tid,
                  thread_tid_t new_thread_tid);

/// Switch to the idle thread.
///
/// Will panic on failure.
void __attribute__((noreturn)) thread_switch_idle(void);

/// Signature that all thread entrypoint functions must obey.
typedef void (*thread_entrypoint_t)(void);

/// Initialize threading.
///
/// Only call this once.
void thread_threading_init(void);

/// Create a new thread.
///
/// Returns `-1` if thread could not be created due to lack of resources.
///
/// Returns `0` on success.
///
/// The new thread's TID is written into the outparam.
int thread_create(thread_entrypoint_t entry, thread_tid_t *tid);

/// Destroy the thread and reclaim it's resources.
///
/// Returns `-1` if a thread with given ID does not exist.
///
/// Returns `0` on success.
int thread_destroy(thread_tid_t tid);

/// Dumps the thread content with kprintf.
///
/// panics when thread does not exists.
void thread_dump_state(thread_tid_t tid);
