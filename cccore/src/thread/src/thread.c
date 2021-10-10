#include "../include/thread.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "../../common.h"
#include "thread/src/tcb.h"

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

/// Currently active thread.
static thread_tid_t THREADS_ACTIVE_TID = 0;

/// TID of the idle thread
static const thread_tid_t THREAD_IDLE_TID = 0;

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

/// Find the thread by it's TID.
///
/// Returns `-1` if no thread with the given TID exists (anymore).
///
/// Returns `0` on success.
static int find_thread_by_tid(thread_tid_t tid, struct thread_t** thread) {
    for (size_t i = 0; i < THREADS_NUM; i++) {
        if (THREADS[i].occupied && THREADS[i].tcb.tid == tid) {
            *thread = &THREADS[i];
            return 0;
        }
    }

    *thread = NULL;
    return -1;
}

/// Save the thread's CPU state to the thread's stack and TCB.
static void save_cpu_state(struct thread_cpu_state_t cpu, struct thread_t* t) {
    unsigned char* dest = (unsigned char*)(t->tcb.stack_ptr) - sizeof(struct thread_cpu_state_t);
    memcpy(dest, &cpu, sizeof(struct thread_cpu_state_t));
    t->tcb.stack_ptr = dest;
}

static void prepare_stack(thread_register_t pc, thread_register_t sp, struct thread_t* t) {
    const struct thread_cpu_state_t cpu = {
        .rip = pc,
        .rax = 1,
        .rbx = 2,
        .rcx = 3,
        .rdx = 4,
        .rsi = 5,
        .rdi = 6,
        .rsp = sp,
        .rbp = sp,  // FIXME: This causes thread to do weird things when it has no more instructions
        .r8 = 7,
        .r9 = 8,
        .r10 = 9,
        .r11 = 10,
        .r12 = 11,
        .r13 = 12,
        .r14 = 13,
        .r15 = 14,
    };
    save_cpu_state(cpu, t);
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
    const struct thread_tcb_t tcb = {
        .stack_ptr = t->stack + THREAD_STACK_SIZE, .state = THREAD_STATE_BLOCKED, .tid = *tid};
    t->tcb = tcb;

    // Make it look to the thread like it's returning after a context switch to eliminate special cases.
    const thread_register_t pc =
        (thread_register_t)entry;  // TODO: Change once user address space != kernel address space
    const thread_register_t sp = (thread_register_t)t->stack +
                                 THREAD_STACK_SIZE;  // TODO: Change once user address space != kernel address space
    prepare_stack(pc, sp, t);
    kprintf("Reached?\n");

    return 0;
}

static void idle_thread(void) {
    while (true) {
        kprintf("Idle thread is running\n");
    }
}

void thread_threading_init(void) {
    // Create the idle thread
    thread_tid_t idle_tid;
    thread_create(idle_thread, &idle_tid);
}

void thread_start_idle(void) {}
static void dump_cpu_state(struct thread_cpu_state_t* cpu_state) {
    kprintf("rip: %x\n", cpu_state->rip);
    kprintf("rax: %x\n", cpu_state->rax);
    kprintf("rbx: %x\n", cpu_state->rbx);
    kprintf("rcx: %x\n", cpu_state->rcx);
    kprintf("rdx: %x\n", cpu_state->rdx);
    kprintf("rsi: %x\n", cpu_state->rsi);
    kprintf("rdi: %x\n", cpu_state->rdi);
    kprintf("rsp: %x\n", cpu_state->rsp);
    kprintf("rbp: %x\n", cpu_state->rbp);
    kprintf("r8:  %x\n", cpu_state->r8);
    kprintf("r9:  %x\n", cpu_state->r9);
    kprintf("r10: %x\n", cpu_state->r10);
    kprintf("r11: %x\n", cpu_state->r11);
    kprintf("r12: %x\n", cpu_state->r12);
    kprintf("r13: %x\n", cpu_state->r13);
    kprintf("r14: %x\n", cpu_state->r14);
    kprintf("r15: %x\n", cpu_state->r15);
}

void thread_dump_state(thread_tid_t tid) {
    struct thread_t* t;
    if (find_thread_by_tid(tid, &t) != 0) {
        kpanicf("tried to dump tid: %d\n", tid);
    }
    kprintf("tcb.stack_ptr: %x\n", t->tcb.stack_ptr);
    struct thread_cpu_state_t* cpu_state = (struct thread_cpu_state_t*)t->tcb.stack_ptr;
    dump_cpu_state(cpu_state);
}

extern void __attribute__((noreturn)) thread_switch_asm(unsigned char* thread_stack_top);

int thread_switch(struct thread_cpu_state_t old_thread_cpu_state, thread_tid_t old_thread_tid,
                  thread_tid_t new_thread_tid) {
    // Stack the old thread's registers
    struct thread_t* old_thread;
    if (find_thread_by_tid(old_thread_tid, &old_thread) != 0) {
        return -1;
    }
    // TODO: Change once user address space != kernel address space
    struct thread_t* old_thread_stack_top = (struct thread_t*)old_thread_cpu_state.rsp;
    save_cpu_state(old_thread_cpu_state, old_thread_stack_top);

    // The hard part
    struct thread_t* new_thread;
    if (find_thread_by_tid(new_thread_tid, &new_thread) != 0) {
        return -1;
    }
    THREADS_ACTIVE_TID = new_thread_tid;

    // TODO: Change once user address space != kernel address space
    unsigned char* new_thread_stack_top = (unsigned char*)new_thread->tcb.stack_ptr;
    thread_switch_asm(new_thread_stack_top);
}

void __attribute__((noreturn)) thread_switch_idle(void) {
    struct thread_t* idle = &THREADS[THREAD_IDLE_TID];
    idle->tcb.state = THREAD_STATE_RUNNING;
    THREADS_ACTIVE_TID = THREAD_IDLE_TID;
    thread_switch_asm(idle->tcb.stack_ptr);
}
