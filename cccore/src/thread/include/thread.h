#pragma once

#include "../src/tcb.h"

/// Signature that all custom ISRs must obey.
///
/// Writing to the struct will modify the respective register on ISR exit.
typedef void (*thread_entrypoint_t)(void);

/// Create a new thread.
///
/// Returns `-1` if thread could not be created due to lack of resources.
///
/// Returns `0` on success.
///
/// The new thread's TID is written into the outparam.
int thread_create(thread_entrypoint_t entry, thread_tid_t *tid);

/// Destroy the thread and reclaim it's resources.
///
/// Returns `-1` if a thread with given ID does not exist.
///
/// Returns `0` on success.
int thread_destroy(thread_tid_t tid);
