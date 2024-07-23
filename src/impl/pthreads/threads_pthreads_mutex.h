#pragma once

#include "config.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>

typedef pthread_mutex_t thread_internal_mutex_t;

#define THREAD_INTERNAL_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#if defined(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP)
#define THREAD_INTERNAL_RECURSIVE_MUTEX_INITIALIZER                            \
  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#elif defined(PTHREAD_RECURSIVE_MUTEX_INITIALIZER)
#define THREAD_INTERNAL_RECURSIVE_MUTEX_INITIALIZER                            \
  PTHREAD_RECURSIVE_MUTEX_INITIALIZER
#endif

static inline int thread_internal_mutex_init(thread_internal_mutex_t *p_mutex,
                                             bool recursive) {
  int ret;
#if ENABLE_DEBUG
  if (recursive) {
    pthread_mutexattr_t mutex_attr;
    ret = pthread_mutexattr_init(&mutex_attr);
    if (0 != ret)
      return ERR_IN_ERRNO;
    ret = pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    if (0 != ret) {
      ret = pthread_mutexattr_destroy(&mutex_attr);
      assert(0 == ret);
      return ERR_IN_ERRNO;
    }
    ret = pthread_mutex_init(p_mutex, &mutex_attr);
    if (0 != ret) {
      ret = pthread_mutexattr_destroy(&mutex_attr);
      assert(0 == ret);
      return ERR_IN_ERRNO;
    }
    ret = pthread_mutexattr_destroy(&mutex_attr);
    assert(0 == ret);
  } else {
    ret = pthread_mutex_init(p_mutex, NULL);
  }
#else
  if (recursive) {
    pthread_mutexattr_t mutex_attr;
    ret = pthread_mutexattr_init(&mutex_attr);
    if (0 != ret) {
      return ERR_IN_ERRNO;
    }
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    ret = pthread_mutex_init(p_mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);
  } else {
    ret = pthread_mutex_init(p_mutex, NULL);
  }
#endif
  return 0 == ret ? SUCCESS : ERR_IN_ERRNO;
}

static inline void
thread_internal_mutex_lock(thread_internal_mutex_t *p_mutex) {
#if ENABLE_DEBUG
  int ret = pthread_mutex_lock(p_mutex);
  if (EDEADLK == ret) {
    show_help("help-opal-threads.txt", "mutex lock failed", true);
  }
  assert(0 == ret);
#else
  pthread_mutex_lock(p_mutex);
#endif
}

static inline int
thread_internal_mutex_trylock(thread_internal_mutex_t *p_mutex) {
  int ret = pthread_mutex_trylock(p_mutex);
  return 0 == ret ? 0 : 1;
}

static inline void
thread_internal_mutex_unlock(thread_internal_mutex_t *p_mutex) {
#if ENABLE_DEBUG
  int ret = pthread_mutex_unlock(p_mutex);
  assert(0 == ret);
#else
  pthread_mutex_unlock(p_mutex);
#endif
}

static inline void
thread_internal_mutex_destroy(thread_internal_mutex_t *p_mutex) {
#if ENABLE_DEBUG
  int ret = pthread_mutex_destroy(p_mutex);
  assert(0 == ret);
#else
  pthread_mutex_destroy(p_mutex);
#endif
}

typedef pthread_cond_t thread_internal_cond_t;

#define THREAD_INTERNAL_COND_INITIALIZER PTHREAD_COND_INITIALIZER

static inline int thread_internal_cond_init(thread_internal_cond_t *p_cond) {
  int ret = pthread_cond_init(p_cond, NULL);
  return 0 == ret ? SUCCESS : ERR_IN_ERRNO;
}

static inline void thread_internal_cond_wait(thread_internal_cond_t *p_cond,
                                             thread_internal_mutex_t *p_mutex) {
#if ENABLE_DEBUG
  int ret = pthread_cond_wait(p_cond, p_mutex);
  assert(0 == ret);
#else
  pthread_cond_wait(p_cond, p_mutex);
#endif
}

static inline void
thread_internal_cond_broadcast(thread_internal_cond_t *p_cond) {
#if ENABLE_DEBUG
  int ret = pthread_cond_broadcast(p_cond);
  assert(0 == ret);
#else
  pthread_cond_broadcast(p_cond);
#endif
}

static inline void thread_internal_cond_signal(thread_internal_cond_t *p_cond) {
#if ENABLE_DEBUG
  int ret = pthread_cond_signal(p_cond);
  assert(0 == ret);
#else
  pthread_cond_signal(p_cond);
#endif
}

static inline void
thread_internal_cond_destroy(thread_internal_cond_t *p_cond) {
#if ENABLE_DEBUG
  int ret = pthread_cond_destroy(p_cond);
  assert(0 == ret);
#else
  pthread_cond_destroy(p_cond);
#endif
}
