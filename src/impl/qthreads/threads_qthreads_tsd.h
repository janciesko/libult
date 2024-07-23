#pragma once

#include "threads_qthreads.h"

typedef qthread_key_t tsd_key_t;

static inline int tsd_key_delete(tsd_key_t key) {
  return 0 == qthread_key_delete(key) ? SUCCESS : ERROR;
}

static inline int tsd_set(tsd_key_t key, void *value) {
  return 0 == qthread_setspecific(key, value) ? SUCCESS : ERROR;
}

static inline int tsd_get(tsd_key_t key, void **valuep) {
  *valuep = qthread_getspecific(key);
  return SUCCESS;
}
