#pragma once

//! This header is the entrypoint to the scheduling system.
//! It currently exposes only a simple round-robin scheduler.

#include "../../include/thread.h"

/// Calculate the next ready thread to run in a round-robin fashion.
///
/// If no threads are ready, returns the idle thread.
thread_tid_t thread_sched_round_robin(void);
