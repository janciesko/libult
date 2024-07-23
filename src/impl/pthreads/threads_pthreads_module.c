
#include <unistd.h>
#include "threads.h"
#include "tsd.h"


int tsd_key_create(tsd_key_t *key, tsd_destructor_t destructor)
{
    int rc;
    rc = pthread_key_create(key, destructor);
    return 0 == rc ? SUCCESS : ERR_IN_ERRNO;
}
