
#include "threads_argobots.h"
#include "thread.h"
#include "threads.h"

static int opal_threads_argobots_open(void);

int opal_threads_argobots_open(void)
{
    opal_threads_argobots_ensure_init();
    return OPAL_SUCCESS;
}
