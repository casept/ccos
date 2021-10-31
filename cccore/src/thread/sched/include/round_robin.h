#pragma once

#include "../../include/thread.h"

/// Calculate the next ready thread to run in a round-robin fashion.
///
/// If no threads are ready, returns the idle thread.
thread_tid_t thread_round_robin_sched(void);
