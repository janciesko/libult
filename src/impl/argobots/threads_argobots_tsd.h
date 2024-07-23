#include "threads_argobots.h"

typedef ABT_key tsd_key_t;

static inline int tsd_key_delete(tsd_key_t key) {
  int ret = ABT_key_free(&key);
  return ABT_SUCCESS == ret ? SUCCESS : ERROR;
}

static inline int tsd_set(tsd_key_t key, void *value) {
  threads_argobots_ensure_init();
  int ret = ABT_key_set(key, value);
  return ABT_SUCCESS == ret ? SUCCESS : ERROR;
}

static inline int tsd_get(tsd_key_t key, void **valuep) {
  int ret = ABT_key_get(key, valuep);
  return ABT_SUCCESS == ret ? SUCCESS : ERROR;
}

#endif /* MCA_THREADS_ARGOBOTS_THREADS_ARGOBOTS_TSD_H */
