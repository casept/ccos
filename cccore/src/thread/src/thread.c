#include "../include/thread.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "../../common.h"
#include "internal.h"
#include "interrupt.h"
#include "tcb.h"
#include "thread/src/tcb.h"

struct thread_t THREADS[THREADS_NUM];

/// TID generator state.
static thread_tid_t THREADS_NEXT_TID = 0;

/// Currently active thread.
thread_tid_t THREADS_ACTIVE_TID = 0;

/// TID of the idle thread
thread_tid_t THREADS_IDLE_TID = 0;

/// Find the first unoccupied thread slot.
///
/// If none are found, return `-1`.
///
/// Return `0` on success.
static int find_free_slot(size_t* slot) {
    for (size_t i = 0; i < THREADS_NUM; i++) {
        if (!THREADS[i].occupied) {
            *slot = i;
            return 0;
        }
    }

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
    t->tcb.state = THREAD_STATE_READY;
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

struct thread_tcb_t* thread_lookup_tcb_by_tid(thread_tid_t tid) {
    for (size_t i = 0; i < THREADS_NUM; i++) {
        if (THREADS[i].tcb.tid == tid) {
            return &THREADS[i].tcb;
        }
    }

    return NULL;
}

int thread_lookup_idx_by_tid(thread_tid_t tid, size_t* idx) {
    for (size_t i = 0; i < THREADS_NUM; i++) {
        if (THREADS[i].tcb.tid == tid) {
            *idx = i;
            return 0;
        }
    }

    return -1;
}

void thread_start_idle(void) { kpanicf("%s: Not implemented", __func__); }

void kprintf_cpu_state_t(const struct thread_cpu_state_t* const cpu_state) {
    kprintf("==== CPU State ====\n");
    kprintf("rip: %p\n", cpu_state->rip);
    kprintf("rax: %p\n", cpu_state->rax);
    kprintf("rbx: %p\n", cpu_state->rbx);
    kprintf("rcx: %p\n", cpu_state->rcx);
    kprintf("rdx: %p\n", cpu_state->rdx);
    kprintf("rsi: %p\n", cpu_state->rsi);
    kprintf("rdi: %p\n", cpu_state->rdi);
    kprintf("rsp: %p\n", cpu_state->rsp);
    kprintf("rbp: %p\n", cpu_state->rbp);
    kprintf("r8:  %p\n", cpu_state->r8);
    kprintf("r9:  %p\n", cpu_state->r9);
    kprintf("r10: %p\n", cpu_state->r10);
    kprintf("r11: %p\n", cpu_state->r11);
    kprintf("r12: %p\n", cpu_state->r12);
    kprintf("r13: %p\n", cpu_state->r13);
    kprintf("r14: %p\n", cpu_state->r14);
    kprintf("r15: %p\n", cpu_state->r15);
    kprintf("===================\n");
}

void thread_dump_state(thread_tid_t tid) {
    struct thread_tcb_t* t = thread_lookup_tcb_by_tid(tid);
    if (t == NULL) {
        kpanicf("tried to dump nonexistent thread with TID: %d\n", tid);
    }
    kprintf("==== Thread State ====\n");
    kprintf("tcb.stack_ptr: 0x%p\n", t->stack_ptr);
    kprintf("tcb.state: ");
    kprintf_thread_state_t(t->state);
    kprintf("\n");
    kprintf("===================\n");
    const struct thread_cpu_state_t* const cpu_state = (struct thread_cpu_state_t*)t->stack_ptr;
    kprintf_cpu_state_t(cpu_state);
}

static void load_cpu_state(const struct thread_cpu_state_t t_cpu, struct interrupt_isr_data_t* isr_data) {
    isr_data->rax = t_cpu.rax;
    isr_data->rbx = t_cpu.rbx;
    isr_data->rcx = t_cpu.rcx;
    isr_data->rdx = t_cpu.rdx;
    isr_data->r8 = t_cpu.r8;
    isr_data->r9 = t_cpu.r9;
    isr_data->r10 = t_cpu.r10;
    isr_data->r11 = t_cpu.r11;
    isr_data->r12 = t_cpu.r12;
    isr_data->r13 = t_cpu.r13;
    isr_data->r14 = t_cpu.r14;
    isr_data->r15 = t_cpu.r15;
    isr_data->rdi = t_cpu.rdi;
    isr_data->rsi = t_cpu.rsi;
    isr_data->rbp = t_cpu.rbp;
    isr_data->rip = t_cpu.rip;
    isr_data->rsp = t_cpu.rsp;
    // FIXME: Restore RFLAGS
}

int thread_switch_prepare(struct thread_cpu_state_t old_thread_cpu_state, thread_tid_t old_thread_tid,
                          thread_tid_t new_thread_tid, struct interrupt_isr_data_t* isr_data) {
    // Stack the old thread's registers
    struct thread_t* old_thread = lookup_thread_by_tid(old_thread_tid);
    if (old_thread == NULL) {
        return -1;
    }
    // TODO: Change once user address space != kernel address space
    struct thread_t* old_thread_stack_top = (struct thread_t*)old_thread_cpu_state.rsp;
    save_cpu_state(old_thread_cpu_state, old_thread_stack_top);

    struct thread_t* new_thread = lookup_thread_by_tid(new_thread_tid);
    if (new_thread == NULL) {
        return -1;
    }
    THREADS_ACTIVE_TID = new_thread_tid;

    // TODO: Change once user address space != kernel address space
    unsigned char* new_thread_stack_top = (unsigned char*)new_thread->tcb.stack_ptr;
    struct thread_cpu_state_t* new_thread_cpu_state = (struct thread_cpu_state_t*)new_thread_stack_top;
    // TODO: Is this the correct location on the stack?
    // The part where we actually write to where the ISR loads them
    load_cpu_state(*new_thread_cpu_state, isr_data);
    return 0;
}

extern void __attribute__((noreturn)) thread_switch_asm(unsigned char* thread_stack_top);

void __attribute__((noreturn)) thread_go(void) {
    struct thread_t* idle = &THREADS[THREADS_IDLE_TID];
    idle->tcb.state = THREAD_STATE_RUNNING;
    THREADS_ACTIVE_TID = THREADS_IDLE_TID;
    thread_switch_asm(idle->tcb.stack_ptr);
}

thread_tid_t thread_get_current_tid(void) { return THREADS_ACTIVE_TID; }
