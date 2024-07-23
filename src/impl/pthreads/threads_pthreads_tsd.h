#pragma once

#include <pthread.h>
#include <signal.h>

typedef pthread_key_t tsd_key_t;

static inline int tsd_key_delete(tsd_key_t key) {
  int ret = pthread_key_delete(key);
  return 0 == ret ? SUCCESS : ERR_IN_ERRNO;
}

static inline int tsd_set(tsd_key_t key, void *value) {
  int ret = pthread_setspecific(key, value);
  return 0 == ret ? SUCCESS : ERR_IN_ERRNO;
}

static inline int tsd_get(tsd_key_t key, void **valuep) {
  *valuep = pthread_getspecific(key);
  return SUCCESS;
}
