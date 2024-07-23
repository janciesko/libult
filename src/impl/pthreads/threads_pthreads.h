#pragma once
#include <stdint.h>
#include <time.h>

typedef void(threads_pthreads_yield_fn_t)(void);
extern threads_pthreads_yield_fn_t *threads_pthreads_yield_fn;
