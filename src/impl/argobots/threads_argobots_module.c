#include "config.h"

#include <unistd.h>

#include "opal/constants.h"
#include "opal/mca/threads/argobots/threads_argobots.h"
#include "opal/mca/threads/threads.h"
#include "opal/mca/threads/tsd.h"
#include "opal/prefetch.h"
#include "opal/util/output.h"
#include "opal/util/sys_limits.h"

int tsd_key_create(tsd_key_t *key, tsd_destructor_t destructor)
{
    threads_argobots_ensure_init();
    int rc;
    rc = ABT_key_create(destructor, key);
    return (ABT_SUCCESS == rc) ? SUCCESS : ERROR;
}
