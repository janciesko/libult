#pragma once
#include "config.h"

#include <stdio.h>
#include <string.h>

#include "mutex.h"
#include "threads_argobots.h"

typedef ABT_mutex_memory thread_internal_mutex_t;

#define THREAD_INTERNAL_MUTEX_INITIALIZER ABT_MUTEX_INITIALIZER
#define THREAD_INTERNAL_RECURSIVE_MUTEX_INITIALIZER                            \
  ABT_RECURSIVE_MUTEX_INITIALIZER

static inline int thread_internal_mutex_init(thread_internal_mutex_t *p_mutex,
                                             bool recursive) {
  if (recursive) {
    const ABT_mutex_memory init_mutex = ABT_RECURSIVE_MUTEX_INITIALIZER;
    memcpy(p_mutex, &init_mutex, sizeof(ABT_mutex_memory));
  } else {
    const ABT_mutex_memory init_mutex = ABT_MUTEX_INITIALIZER;
    memcpy(p_mutex, &init_mutex, sizeof(ABT_mutex_memory));
  }
  return SUCCESS;
}

static inline void
thread_internal_mutex_lock(thread_internal_mutex_t *p_mutex) {
  ABT_mutex mutex = ABT_MUTEX_MEMORY_GET_HANDLE(p_mutex);
#if ENABLE_DEBUG
  int ret = ABT_mutex_lock(mutex);
  if (ABT_SUCCESS != ret) {
    show_help("help-opal-threads.txt", "mutex lock failed", true);
  }
#else
  ABT_mutex_lock(mutex);
#endif
}

static inline int
thread_internal_mutex_trylock(thread_internal_mutex_t *p_mutex) {
  ABT_mutex mutex = ABT_MUTEX_MEMORY_GET_HANDLE(p_mutex);
  int ret = ABT_mutex_trylock(mutex);
  if (ABT_ERR_MUTEX_LOCKED == ret) {
    return 1;
  } else if (ABT_SUCCESS != ret) {
#if ENABLE_DEBUG
    show_help("help-opal-threads.txt", "mutex trylock failed", true);
#endif
    return 1;
  }
  return 0;
}

static inline void
thread_internal_mutex_unlock(thread_internal_mutex_t *p_mutex) {
  ABT_mutex mutex = ABT_MUTEX_MEMORY_GET_HANDLE(p_mutex);
#if ENABLE_DEBUG
  int ret = ABT_mutex_unlock(mutex);
  if (ABT_SUCCESS != ret) {
    show_help("help-opal-threads.txt", "mutex unlock failed", true);
  }
#else
  ABT_mutex_unlock(mutex);
#endif
  /* For fairness of locking. */
  ABT_thread_yield();
}

static inline void
thread_internal_mutex_destroy(thread_internal_mutex_t *p_mutex) {
  /* No specific operation is needed to destroy thread_internal_mutex_t. */
}

typedef ABT_cond_memory thread_internal_cond_t;

#define THREAD_INTERNAL_COND_INITIALIZER ABT_COND_INITIALIZER

static inline int thread_internal_cond_init(thread_internal_cond_t *p_cond) {
  const ABT_cond_memory init_cond = ABT_COND_INITIALIZER;
  memcpy(p_cond, &init_cond, sizeof(ABT_cond_memory));
  return SUCCESS;
}

static inline void thread_internal_cond_wait(thread_internal_cond_t *p_cond,
                                             thread_internal_mutex_t *p_mutex) {
  ABT_mutex mutex = ABT_MUTEX_MEMORY_GET_HANDLE(p_mutex);
  ABT_cond cond = ABT_COND_MEMORY_GET_HANDLE(p_cond);
#if ENABLE_DEBUG
  int ret = ABT_cond_wait(cond, mutex);
  assert(ABT_SUCCESS == ret);
#else
  ABT_cond_wait(cond, mutex);
#endif
}

static inline void
thread_internal_cond_broadcast(thread_internal_cond_t *p_cond) {
  ABT_cond cond = ABT_COND_MEMORY_GET_HANDLE(p_cond);
#if ENABLE_DEBUG
  int ret = ABT_cond_broadcast(cond);
  assert(ABT_SUCCESS == ret);
#else
  ABT_cond_broadcast(cond);
#endif
}

static inline void thread_internal_cond_signal(thread_internal_cond_t *p_cond) {
  ABT_cond cond = ABT_COND_MEMORY_GET_HANDLE(p_cond);
#if ENABLE_DEBUG
  int ret = ABT_cond_signal(cond);
  assert(ABT_SUCCESS == ret);
#else
  ABT_cond_signal(cond);
#endif
}

static inline void
thread_internal_cond_destroy(thread_internal_cond_t *p_cond) {
  /* No destructor is needed. */
}