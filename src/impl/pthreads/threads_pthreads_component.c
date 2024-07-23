#include "threads_pthreads.h"
#include "threads.h"

static int threads_pthreads_open(void);
static int threads_pthreads_register(void);

int threads_pthreads_register(void)
{
    return threads_pthreads_yield_init(&mca_threads_pthreads_component.threadsc_version);
}

int threads_pthreads_open(void)
{
    return SUCCESS;
}
