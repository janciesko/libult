#include "config.h"

#include "mutex.h"

/*
 * Wait and see if some upper layer wants to use threads, if support
 * exists.
 */
bool uses_threads = false;

static void mca_threads_mutex_constructor(mutex_t *p_mutex)
{
#if ENABLE_DEBUG
    int ret = thread_internal_mutex_init(&p_mutex->m_lock, false);
    assert(0 == ret);
    p_mutex->m_lock_debug = 0;
    p_mutex->m_lock_file = NULL;
    p_mutex->m_lock_line = 0;
#else
    thread_internal_mutex_init(&p_mutex->m_lock, false);
#endif
    atomic_lock_init(&p_mutex->m_lock_atomic, 0);
}

static void mca_threads_mutex_destructor(mutex_t *p_mutex)
{
    thread_internal_mutex_destroy(&p_mutex->m_lock);
}

static void mca_threads_recursive_mutex_constructor(recursive_mutex_t *p_mutex)
{
#if ENABLE_DEBUG
    int ret = thread_internal_mutex_init(&p_mutex->m_lock, true);
    assert(0 == ret);
    p_mutex->m_lock_debug = 0;
    p_mutex->m_lock_file = NULL;
    p_mutex->m_lock_line = 0;
#else
    thread_internal_mutex_init(&p_mutex->m_lock, true);
#endif
    atomic_lock_init(&p_mutex->m_lock_atomic, 0);
}

static void mca_threads_recursive_mutex_destructor(recursive_mutex_t *p_mutex)
{
    thread_internal_mutex_destroy(&p_mutex->m_lock);
}

OBJ_CLASS_INSTANCE(mutex_t, object_t, mca_threads_mutex_constructor,
                   mca_threads_mutex_destructor);

OBJ_CLASS_INSTANCE(recursive_mutex_t, object_t, mca_threads_recursive_mutex_constructor,
                   mca_threads_recursive_mutex_destructor);

int cond_init(cond_t *cond)
{
    return thread_internal_cond_init(cond);
}

int cond_wait(cond_t *cond, mutex_t *lock)
{
    thread_internal_cond_wait(cond, &lock->m_lock);
    return SUCCESS;
}

int cond_broadcast(cond_t *cond)
{
    thread_internal_cond_broadcast(cond);
    return SUCCESS;
}

int cond_signal(cond_t *cond)
{
    thread_internal_cond_signal(cond);
    return SUCCESS;
}

int cond_destroy(cond_t *cond)
{
    thread_internal_cond_destroy(cond);
    return SUCCESS;
}
