#pragma once

#include "mutex.h"
#include <pthread.h>

/**
 * @file
 *
 * Thread Specific Datastore Interface
 *
 * Functions for providing thread-specific datastore capabilities.
 */

/**
 * Prototype for callback when tsd data is being destroyed
 */
typedef void (*tsd_destructor_t)(void *value);

#if defined(DOXYGEN)

/**
 * Typedef for thread-specific data key
 */
typedef void *tsd_key_t;

/**
 * Delete a thread-specific data key
 *
 * Delete a thread-specific data key previously returned by
 * tsd_key_create().  The destructor associated with the key is
 * not fired in any thread and memory cleanup is the responsibility of
 * the caller.
 *
 * @note Unlike pthread_key_delete, this function should not be called
 * from within a destructor.  It can not be universally supported at
 * this time.
 *
 * @param key[in]       The key for accessing thread-specific data
 *
 * @retval SUCCESS      Success
 * @retval ERROR        Error
 * @retval ERR_IN_ERRNO Error
 */
int tsd_key_delete(tsd_key_t key);

/**
 * Set a thread-specific data value
 *
 * Associates value with key in the current thread.  The value for the
 * key in other threads is not changed.  Different threads may assign
 * different values to the same key.
 *
 * @note This function should not be called within
 * tsd_key_delete().
 *
 * @param key[in]       Thread specific data key to modify
 * @param value[in]     Value to associate with key
 *
 * @retval SUCCESS      Success
 * @retval ERR          Error
 * @retval ERR_IN_ERRNO Error
 */
int tsd_setspecific(tsd_key_t key, void *value);

/**
 * Get a thread-specific data value
 *
 * Get the data associated with the given key, as set by
 * tsd_setspecific().  If tsd_setspecific() hasn't been
 * called in the current thread with the given key, NULL is returned
 * in valuep.
 *
 * @param key[in]        Thread specific data key to modify
 * @param value[out]     Value to associate with key
 *
 * @retval SUCCESS      Success
 * @retval ERR          Error
 * @retval ERR_IN_ERRNO Error
 */
int tsd_getspecific(tsd_key_t key, void **valuep);

#else

#include MCA_threads_tsd_base_include_HEADER

#endif

typedef struct tsd_tracked_key_s tsd_tracked_key_t;

typedef struct _tsd_list_item_t {
  list_item_t super;
  tsd_tracked_key_t *tracked_key;
  void *data;
} tsd_list_item_t;
OBJ_CLASS_DECLARATION(tsd_list_item_t);

struct tsd_tracked_key_s {
  object_t super;
  tsd_key_t key;
  mutex_t mutex;
  list_t tsd_list;
  void (*user_destructor)(void *);
};
OBJ_CLASS_DECLARATION(tsd_tracked_key_t);

void tsd_tracked_key_constructor(tsd_tracked_key_t *key);
void tsd_tracked_key_destructor(tsd_tracked_key_t *key);

static inline int tsd_tracked_key_get(tsd_tracked_key_t *key, void **p) {
  assert(NULL != key);
  *p = NULL;

  tsd_list_item_t *tsd = NULL;
  tsd_get(key->key, (void **)&tsd);
  if (NULL != tsd) {
    *p = tsd->data;
  }

  return SUCCESS;
}

int tsd_tracked_key_set(tsd_tracked_key_t *key, void *p);
void tsd_tracked_key_set_destructor(tsd_tracked_key_t *key,
                                    tsd_destructor_t destructor);
int tsd_key_create(tsd_key_t *key, tsd_destructor_t destructor);
