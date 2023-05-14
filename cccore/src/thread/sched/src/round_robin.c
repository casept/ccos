#include <stddef.h>

#include "../../common.h"
#include "../../include/thread.h"
#include "../../src/internal.h"
#include "../include/sched.h"

thread_tid_t thread_sched_round_robin(void) {
    // Find index of active thread
    size_t curr_thread_idx;
    if (lookup_idx_by_tid(THREADS_ACTIVE_TID, &curr_thread_idx) != 0) {
        kpanicf("%s: Could not locate TCB of currently active thread", __func__);
    }

    // Walk all threads after active one
    for (size_t i = curr_thread_idx + 1; i < (THREADS_NUM - curr_thread_idx); i++) {
        if (THREADS[i].tcb.state == THREAD_STATE_READY) {
            return THREADS[i].tcb.tid;
        }
    }

    // Walk all threads before active one
    for (size_t i = 0; i <= curr_thread_idx; i++) {
        if (THREADS[i].tcb.state == THREAD_STATE_READY) {
            return THREADS[i].tcb.tid;
        }
    }

    // Return idle thread if no ready thread could be found
    kprintf("%s: using idle thread as backup\n", __func__);
    return THREADS_IDLE_TID;
}
