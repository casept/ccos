#pragma once

#include <stdint.h>

typedef uint64_t thread_tid_t;

enum thread_state_t {
    THREAD_STATE_RUNNING,
    THREAD_STATE_READY,
    THREAD_STATE_DEAD,
    THREAD_STATE_BLOCKED,
};
void kprintf_thread_state_t(enum thread_state_t state);

struct thread_tcb_t {
    thread_tid_t tid;
    void *stack_ptr;
    enum thread_state_t state;
};

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
