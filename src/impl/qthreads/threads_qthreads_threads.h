#pragma once

#include "qthreads/threads_qthreads.h"

/* Qthreads are cooperatively scheduled so yield when idle */
#define THREAD_YIELD_WHEN_IDLE_DEFAULT true

static inline void thread_yield(void) { qthread_yield(); }
