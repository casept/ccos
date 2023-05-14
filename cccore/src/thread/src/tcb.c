#include "tcb.h"

#include "../../common.h"

void kprintf_thread_state_t(enum thread_state_t state) {
    switch (state) {
        case THREAD_STATE_RUNNING:
            kprintf("THREAD_STATE_RUNNING");
            break;
        case THREAD_STATE_READY:
            kprintf("THREAD_STATE_READY");
            break;
        case THREAD_STATE_DEAD:
            kprintf("THREAD_STATE_DEAD");
            break;
        case THREAD_STATE_BLOCKED:
            kprintf("THREAD_STATE_BLOCKED");
            break;
        default:
            kprintf("UNKNOWN THREAD STATE");
            break;
    }
}