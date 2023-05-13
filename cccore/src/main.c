#include <stdbool.h>
#include <stddef.h>

#include "common.h"
#include "hal/include/exception.h"
#include "hal/include/gdt.h"
#include "hal/include/interrupt.h"
#include "hal/include/timer.h"
#include "stivale2.h"
#include "tcb.h"
#include "thread/include/thread.h"
#include "thread/sched/include/sched.h"
#include "thread/src/internal.h"

#define STACK_SIZE 4096
static uint8_t stack[STACK_SIZE];

__attribute__((section(".stivale2hdr"), used)) static struct stivale2_header stivale_hdr = {
    .entry_point = 0,                           // Leave ELF default
    .stack = (uintptr_t)stack + sizeof(stack),  // Stack grows downwards
    .flags = (1 << 1) | (1 << 2),               // Higher half w/ MMU configured as in linker script
    .tags = 0,                                  // No tags means the bootloader will give us CGA text mode or bust
};

void kmain(void);

// NOLINTNEXTLINE (bugprone-reserved-identifier)
void _start(struct stivale2_struct *config) {
    (void)config;  // Don't care about stivale data for now

    // TODO: Zero out .bss
    kmain();
    while (true) {
    }
}

static struct thread_cpu_state_t isr_data_2_cpu_state(const struct interrupt_isr_data_t *const data) {
    struct thread_cpu_state_t t_cpu;
    t_cpu.rax = data->rax;
    t_cpu.rbx = data->rbx;
    t_cpu.rcx = data->rcx;
    t_cpu.rdx = data->rdx;
    t_cpu.r8 = data->r8;
    t_cpu.r9 = data->r9;
    t_cpu.r10 = data->r10;
    t_cpu.r11 = data->r11;
    t_cpu.r12 = data->r12;
    t_cpu.r13 = data->r13;
    t_cpu.r14 = data->r14;
    t_cpu.r15 = data->r15;
    t_cpu.rdi = data->rdi;
    t_cpu.rsi = data->rsi;
    t_cpu.rbp = data->rbp;
    t_cpu.rip = data->rip;
    t_cpu.rsp = data->rsp;

    return t_cpu;
}

static void timer(struct interrupt_isr_data_t *isr_data) {
    kprintf("Tick\n");
    // Convert CPU state to format expected by thread subsystem
    struct thread_cpu_state_t t_cpu = isr_data_2_cpu_state(isr_data);

    // TODO: Let thread switcher load registers as expected
    // For now, just letting the old process run again is good enough

    // Let scheduler pick next thread to run
    const thread_tid_t tid_new = thread_sched_round_robin();
    const thread_tid_t tid_old = thread_get_current_tid();
    // TODO: Remove
    kprintf("State of old thread %lu:\n", tid_old);
    thread_dump_state(tid_old);
    kprintf("State of new thread %lu:\n", tid_new);
    thread_dump_state(tid_new);
    thread_switch_prepare(t_cpu, tid_old, tid_new, isr_data);

    // Return from ISR, causing the interrupt handler to load the registers
    return;
}

/*
void test_thread_1(void) {
    while (true) {
        asm volatile("");
    }
}
void test_thread_2(void) {
    while (true) {
        asm volatile("");
    }
}
*/

void kmain(void) {
    interrupt_disable();
    kprint_init();
    kprintf("Hello\n");

    gdt_init_flat();
    interrupt_init();
    exception_register_default();

    timer_enable(1, timer);
    interrupt_enable();

    thread_threading_init();
    /*
    thread_tid_t test_1_tid;
    thread_create(test_thread_1, &test_1_tid);
    thread_tid_t test_2_tid;
    thread_create(test_thread_2, &test_2_tid);
    */
    kprintf("Starting idle thread!\n");
    thread_go();
}
