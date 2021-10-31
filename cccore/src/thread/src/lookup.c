#include <stddef.h>

#include "../../common.h"
#include "../include/thread.h"
#include "internal.h"
#include "thread/src/tcb.h"

struct thread_t* lookup_thread_by_tid(thread_tid_t tid) {
    for (size_t i = 0; i < THREADS_NUM; i++) {
        if (THREADS[i].tcb.tid == tid) {
            return &THREADS[i];
        }
    }

    return NULL;
}

int lookup_idx_by_tid(thread_tid_t tid, size_t* idx) {
    for (size_t i = 0; i < THREADS_NUM; i++) {
        if (THREADS[i].tcb.tid == tid) {
            *idx = i;
            return 0;
        }
    }

    return -1;
}
