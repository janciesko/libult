#pragma once

#include "threads_argobots.h"

/* Argobots are cooperatively scheduled so yield when idle */
#define OPAL_THREAD_YIELD_WHEN_IDLE_DEFAULT true

static inline void opal_thread_yield(void) { ABT_thread_yield(); }
