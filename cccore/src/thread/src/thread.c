#include "../include/thread.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "switch.h"

#define THREAD_STACK_SIZE 4096
#define THREADS_NUM 16

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
struct thread_t THREADS[THREADS_NUM];

/// TID generator state.
static thread_tid_t THREADS_NEXT_TID = 0;

/// Finds the first unoccupied thread slot.
///
/// If none are found, returns `-1`.
///
/// Returns `0` on success.
static int find_free_slot(size_t* slot) {
    for (size_t i = 0; i < THREADS_NUM; i++) {
        if (!THREADS[i].occupied) {
            *slot = i;
            return 0;
        }
    }

    return -1;
}

static void prepare_stack(thread_register_t pc, thread_register_t sp, unsigned char* stack_top) {
    const struct thread_cpu_state_t cpu = {
        .rip = pc,
        .rax = 0,
        .rbx = 0,
        .rcx = 0,
        .rdx = 0,
        .rsi = 0,
        .rdi = 0,
        .rsp = sp,
        .rbp = sp,
        .r8 = 0,
        .r9 = 0,
        .r10 = 0,
        .r11 = 0,
        .r12 = 0,
        .r13 = 0,
        .r14 = 0,
        .r15 = 0,
    };

    unsigned char* dest = stack_top - sizeof(struct thread_cpu_state_t);
    memcpy(dest, &cpu, sizeof(struct thread_cpu_state_t));
}

int thread_create(thread_entrypoint_t entry, thread_tid_t* tid) {
    // Which slot is free, if any?
    size_t slot;
    if (find_free_slot(&slot) != 0) {
        return -1;
    }
    struct thread_t* t = &THREADS[slot];

    // Allocate a TID.
    // For now, just increment a global counter.
    *tid = THREADS_NEXT_TID;
    THREADS_NEXT_TID++;

    // Prepare data structures
    t->occupied = true;
    memset(t->stack, 0, THREAD_STACK_SIZE);
    const struct thread_tcb_t tcb = {.stack_ptr = t->stack, .state = THREAD_STATE_READY, .tid = *tid};
    t->tcb = tcb;

    // Make it look to the thread like it's returning after a context switch to eliminate special cases.
    const thread_register_t pc =
        (thread_register_t)entry;  // TODO: Change once user address space != kernel address space
    const thread_register_t sp = (thread_register_t)t->stack +
                                 THREAD_STACK_SIZE;  // TODO: Change once user address space != kernel address space
    prepare_stack(pc, sp, t->stack);

    return 0;
}
