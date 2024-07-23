
#pragma once
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <pthread.h>
#include <time.h>

#include "mutex.h"

/*
 * Combine pthread support w/ polled progress to allow run-time selection
 * of threading vs. non-threading progress.
 */

struct condition_t {
  object_t super;
  volatile int c_waiting;
  volatile int c_signaled;
};
typedef struct condition_t condition_t;

DECLSPEC OBJ_CLASS_DECLARATION(condition_t);

static inline int condition_wait(condition_t *c, mutex_t *m) {
  int rc = SUCCESS;
  c->c_waiting++;

  if (using_threads()) {
    if (c->c_signaled) {
      c->c_waiting--;
      mutex_unlock(m);
      progress();
      mutex_lock(m);
      return rc;
    }
    while (0 == c->c_signaled) {
      mutex_unlock(m);
      progress();
      mutex_lock(m);
    }
  } else {
    while (0 == c->c_signaled) {
      progress();
    }
  }

  c->c_signaled--;
  c->c_waiting--;
  return rc;
}

static inline int condition_timedwait(condition_t *c, mutex_t *m,
                                      const struct timespec *abstime) {
  struct timeval tv;
  struct timeval absolute;
  int rc = SUCCESS;

  c->c_waiting++;
  if (using_threads()) {
    absolute.tv_sec = abstime->tv_sec;
    absolute.tv_usec = abstime->tv_nsec / 1000;
    gettimeofday(&tv, NULL);
    if (0 == c->c_signaled) {
      do {
        mutex_unlock(m);
        progress();
        gettimeofday(&tv, NULL);
        mutex_lock(m);
      } while (0 == c->c_signaled && (tv.tv_sec <= absolute.tv_sec ||
                                      (tv.tv_sec == absolute.tv_sec &&
                                       tv.tv_usec < absolute.tv_usec)));
    }
  } else {
    absolute.tv_sec = abstime->tv_sec;
    absolute.tv_usec = abstime->tv_nsec / 1000;
    gettimeofday(&tv, NULL);
    if (0 == c->c_signaled) {
      do {
        progress();
        gettimeofday(&tv, NULL);
      } while (0 == c->c_signaled && (tv.tv_sec <= absolute.tv_sec ||
                                      (tv.tv_sec == absolute.tv_sec &&
                                       tv.tv_usec < absolute.tv_usec)));
    }
  }

  if (0 != c->c_signaled) {
    c->c_signaled--;
  }
  c->c_waiting--;
  return rc;
}

static inline int condition_signal(condition_t *c) {
  if (c->c_waiting) {
    c->c_signaled++;
  }
  return SUCCESS;
}

static inline int condition_broadcast(condition_t *c) {
  c->c_signaled = c->c_waiting;
  return SUCCESS;
}
