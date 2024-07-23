#pragma once

#include <pthread.h>
#include <signal.h>

#include "hreads_pthreads.h"
#include "threads.h"

/* Pthreads do not need to yield when idle */
#define THREAD_YIELD_WHEN_IDLE_DEFAULT false

static inline void thread_yield(void) { threads_pthreads_yield_fn(); }
