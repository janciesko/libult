
#ifndef MCA_THREADS_QTHREADS_THREADS_QTHREADS_MUTEX_H
#define MCA_THREADS_QTHREADS_THREADS_QTHREADS_MUTEX_H 1

#include "config.h"

#include "threads_qthreads.h"
#include <stdio.h>

typedef qthread_spinlock_t thread_internal_mutex_t;

#define THREAD_INTERNAL_MUTEX_INITIALIZER QTHREAD_MUTEX_INITIALIZER
#define THREAD_INTERNAL_RECURSIVE_MUTEX_INITIALIZER                            \
  QTHREAD_RECURSIVE_MUTEX_INITIALIZER

static inline int thread_internal_mutex_init(thread_internal_mutex_t *p_mutex,
                                             bool recursive) {
  threads_ensure_init_qthreads();
#if ENABLE_DEBUG
  int ret = qthread_spinlock_init(p_mutex, recursive);
  if (QTHREAD_SUCCESS != ret) {
    show_help("help-opal-threads.txt", "mutex init failed", true);
  }
#else
  qthread_spinlock_init(p_mutex, recursive);
#endif
  return SUCCESS;
}

static inline void
thread_internal_mutex_lock(thread_internal_mutex_t *p_mutex) {
  threads_ensure_init_qthreads();
#if ENABLE_DEBUG
  int ret = qthread_spinlock_lock(p_mutex);
  if (QTHREAD_SUCCESS != ret) {
    show_help("help-opal-threads.txt", "mutex lock failed", true);
  }
#else
  qthread_spinlock_lock(p_mutex);
#endif
}

static inline int
thread_internal_mutex_trylock(thread_internal_mutex_t *p_mutex) {
  threads_ensure_init_qthreads();
  int ret = qthread_spinlock_trylock(p_mutex);
  if (QTHREAD_OPFAIL == ret) {
    return 1;
  } else if (QTHREAD_SUCCESS != ret) {
#if ENABLE_DEBUG
    show_help("help-opal-threads.txt", "mutex trylock failed", true);
#endif
    return 1;
  }
  return 0;
}

static inline void
thread_internal_mutex_unlock(thread_internal_mutex_t *p_mutex) {
  threads_ensure_init_qthreads();
  int ret;
#if ENABLE_DEBUG
  ret = qthread_spinlock_unlock(p_mutex);
  if (QTHREAD_SUCCESS != ret) {
    show_help("help-opal-threads.txt", "mutex unlock failed", true);
  }
#else
  qthread_spinlock_unlock(p_mutex);
#endif
  /* For fairness of locking. */
  qthread_yield();
}

static inline void
thread_internal_mutex_destroy(thread_internal_mutex_t *p_mutex) {
  /* No specific operation is needed to destroy thread_internal_mutex_t. */
}

typedef struct thread_cond_waiter_t {
  int m_signaled;
  struct thread_cond_waiter_t *m_prev;
} thread_cond_waiter_t;

typedef struct {
  thread_internal_mutex_t m_lock;
  thread_cond_waiter_t *m_waiter_head;
  thread_cond_waiter_t *m_waiter_tail;
} thread_internal_cond_t;

#define THREAD_INTERNAL_COND_INITIALIZER                                       \
  {                                                                            \
    .m_lock = QTHREAD_MUTEX_INITIALIZER, .m_waiter_head = NULL,                \
    .m_waiter_tail = NULL,                                                     \
  }

static inline int thread_internal_cond_init(thread_internal_cond_t *p_cond) {
  qthread_spinlock_init(&p_cond->m_lock, false /* is_recursive */);
  p_cond->m_waiter_head = NULL;
  p_cond->m_waiter_tail = NULL;
  return SUCCESS;
}

static inline void thread_internal_cond_wait(thread_internal_cond_t *p_cond,
                                             thread_internal_mutex_t *p_mutex) {
  threads_ensure_init_qthreads();
  /* This thread is taking "lock", so only this thread can access this
   * condition variable.  */
  qthread_spinlock_lock(&p_cond->m_lock);
  thread_cond_waiter_t waiter = {0, NULL};
  if (NULL == p_cond->m_waiter_head) {
    p_cond->m_waiter_tail = &waiter;
  } else {
    p_cond->m_waiter_head->m_prev = &waiter;
  }
  p_cond->m_waiter_head = &waiter;
  qthread_spinlock_unlock(&p_cond->m_lock);
  while (1) {
    thread_internal_mutex_unlock(p_mutex);
    qthread_yield();
    thread_internal_mutex_lock(p_mutex);
    /* Check if someone woke me up. */
    qthread_spinlock_lock(&p_cond->m_lock);
    int signaled = waiter.m_signaled;
    qthread_spinlock_unlock(&p_cond->m_lock);
    if (1 == signaled) {
      break;
    }
    /* Unlock the lock again. */
  }
}

static inline void
thread_internal_cond_broadcast(thread_internal_cond_t *p_cond) {
  qthread_spinlock_lock(&p_cond->m_lock);
  while (NULL != p_cond->m_waiter_tail) {
    thread_cond_waiter_t *p_cur_tail = p_cond->m_waiter_tail;
    p_cond->m_waiter_tail = p_cur_tail->m_prev;
    /* Awaken one of threads in a FIFO manner. */
    p_cur_tail->m_signaled = 1;
  }
  /* No waiters. */
  p_cond->m_waiter_head = NULL;
  qthread_spinlock_unlock(&p_cond->m_lock);
}

static inline void thread_internal_cond_signal(thread_internal_cond_t *p_cond) {
  qthread_spinlock_lock(&p_cond->m_lock);
  if (NULL != p_cond->m_waiter_tail) {
    thread_cond_waiter_t *p_cur_tail = p_cond->m_waiter_tail;
    p_cond->m_waiter_tail = p_cur_tail->m_prev;
    /* Awaken one of threads. */
    p_cur_tail->m_signaled = 1;
    if (NULL == p_cond->m_waiter_tail) {
      p_cond->m_waiter_head = NULL;
    }
  }
  qthread_spinlock_unlock(&p_cond->m_lock);
}

static inline void
thread_internal_cond_destroy(thread_internal_cond_t *p_cond) {
  /* No destructor is needed. */
}
