
#include "threads_qthreads.h"
#include "threads.h"
#include "threads/tsd.h"

struct tsd_key_value {
    tsd_key_t key;
    tsd_destructor_t destructor;
};

/* false: uninitialized, true: initialized. */
static atomic_lock_t thread_self_key_lock = ATOMIC_LOCK_INIT;
static bool thread_self_key_init = false;
static tsd_key_t thread_self_key;

static inline void self_key_ensure_init(void)
{
    if (false == thread_self_key_init) {
        /* not initialized yet. */
        atomic_lock(&thread_self_key_lock);
        /* check again. */
        if (false == thread_self_key_init) {
            /* This thread is responsible for initializing this key. */
            qthread_key_create(&thread_self_key, NULL);
            atomic_mb();
            thread_self_key_init = true;
        }
        atomic_unlock(&thread_self_key_lock);
    }
    /* thread_self_key has been already initialized. */
}

int tsd_key_create(tsd_key_t *key, tsd_destructor_t destructor)
{
    threads_ensure_init_qthreads();
    qthread_key_create(key, destructor);
    return SUCCESS;
}
