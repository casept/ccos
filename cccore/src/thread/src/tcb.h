#pragma once

#include <stdint.h>

typedef uint64_t thread_tid_t;

enum thread_state_t {
    THREAD_STATE_RUNNING,
    THREAD_STATE_READY,
    THREAD_STATE_DEAD,
    THREAD_STATE_BLOCKED,
};

struct thread_tcb_t {
    thread_tid_t tid;
    void *stack_ptr;
    enum thread_state_t state;
};
