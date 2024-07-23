#include <unistd.h>
#include <pthread.h>

#include "threads.h"
#include "tsd.h"

/*
 * Constructor
 */
static void thread_construct(thread_t *t)
{
    t->t_run = 0;
    t->t_handle = (pthread_t) -1;
}

OBJ_CLASS_INSTANCE(thread_t, object_t, thread_construct, NULL);

int thread_start(thread_t *t)
{
    int rc;

    if (ENABLE_DEBUG) {
        if (NULL == t->t_run || (pthread_t) -1 != t->t_handle) {
            return ERR_BAD_PARAM;
        }
    }

    rc = pthread_create(&t->t_handle, NULL, (void *(*) (void *) ) t->t_run, t);

    return 0 == rc ? SUCCESS : ERR_IN_ERRNO;
}

int thread_join(thread_t *t, void **thr_return)
{
    int rc = pthread_join(t->t_handle, thr_return);
    t->t_handle = (pthread_t) -1;
    return 0 == rc ? SUCCESS : ERR_IN_ERRNO;
}

bool thread_self_compare(thread_t *t)
{
    return pthread_self() == t->t_handle;
}

thread_t *thread_get_self(void)
{
    thread_t *t = OBJ_NEW(thread_t);
    t->t_handle = pthread_self();
    return t;
}

void thread_set_main(void)
{
}
