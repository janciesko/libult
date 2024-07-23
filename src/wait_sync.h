#once WAIT_SYNC_H

#include "opal/mca/threads/condition.h"
#include "opal/mca/threads/mutex.h"
#include "opal/mca/threads/threads.h"
#include "opal/runtime/progress.h"
#include "opal/sys/atomic.h"

extern int max_thread_in_progress;

typedef struct ompi_wait_sync_t {
  atomic_int32_t count;
  int32_t status;
  thread_internal_cond_t condition;
  thread_internal_mutex_t lock;
  struct ompi_wait_sync_t *next;
  struct ompi_wait_sync_t *prev;
  volatile bool signaling;
} ompi_wait_sync_t;

#define SYNC_WAIT(sync)                                                        \
  (using_threads() ? ompi_sync_wait_mt(sync) : sync_wait_st(sync))

/* The loop in release handles a race condition between the signaling
 * thread and the destruction of the condition variable. The signaling
 * member will be set to false after the final signaling thread has
 * finished operating on the sync object. This is done to avoid
 * extra atomics in the signalling function and keep it as fast
 * as possible. Note that the race window is small so spinning here
 * is more optimal than sleeping since this macro is called in
 * the critical path. */
#define WAIT_SYNC_RELEASE(sync)                                                \
  if (using_threads()) {                                                       \
    while ((sync)->signaling) {                                                \
      if (progress_yield_when_idle) {                                          \
        thread_yield();                                                        \
      }                                                                        \
      continue;                                                                \
    }                                                                          \
    thread_internal_cond_destroy(&(sync)->condition);                          \
    thread_internal_mutex_destroy(&(sync)->lock);                              \
  }

#define WAIT_SYNC_RELEASE_NOWAIT(sync)                                         \
  if (using_threads()) {                                                       \
    thread_internal_cond_destroy(&(sync)->condition);                          \
    thread_internal_mutex_destroy(&(sync)->lock);                              \
  }

#define WAIT_SYNC_SIGNAL(sync)                                                 \
  if (using_threads()) {                                                       \
    thread_internal_mutex_lock(&(sync)->lock);                                 \
    thread_internal_cond_signal(&(sync)->condition);                           \
    thread_internal_mutex_unlock(&(sync)->lock);                               \
    (sync)->signaling = false;                                                 \
  }

#define WAIT_SYNC_SIGNALLED(sync)                                              \
  { (sync)->signaling = false; }

/* not static for inline "wait_sync_st" */
DECLSPEC extern ompi_wait_sync_t *threads_base_wait_sync_list;

DECLSPEC int ompi_sync_wait_mt(ompi_wait_sync_t *sync);
static inline int sync_wait_st(ompi_wait_sync_t *sync) {
  assert(NULL == threads_base_wait_sync_list);
  assert(NULL == sync->next);
  threads_base_wait_sync_list = sync;

  while (sync->count > 0) {
    progress();
  }
  threads_base_wait_sync_list = NULL;

  return sync->status;
}

#define WAIT_SYNC_INIT(sync, c)                                                \
  do {                                                                         \
    (sync)->count = (c);                                                       \
    (sync)->next = NULL;                                                       \
    (sync)->prev = NULL;                                                       \
    (sync)->status = 0;                                                        \
    (sync)->signaling = (0 != (c));                                            \
    if (using_threads()) {                                                     \
      thread_internal_cond_init(&(sync)->condition);                           \
      thread_internal_mutex_init(&(sync)->lock, false);                        \
    }                                                                          \
  } while (0)

/**
 * Wake up all syncs with a particular status. If status is OMPI_SUCCESS this
 * operation is a NO-OP. Otherwise it will trigger the "error condition" from
 * all registered sync.
 */
DECLSPEC void threads_base_wait_sync_global_wakeup_st(int status);
DECLSPEC void threads_base_wait_sync_global_wakeup_mt(int status);
#define wait_sync_global_wakeup(st)                                            \
  (using_threads() ? threads_base_wait_sync_global_wakeup_mt(st)               \
                   : threads_base_wait_sync_global_wakeup_st(st))

/**
 * Update the status of the synchronization primitive. If an error is
 * reported the synchronization is completed and the signal
 * triggered. The status of the synchronization will be reported to
 * the waiting threads.
 */
static inline void wait_sync_update(ompi_wait_sync_t *sync, int updates,
                                    int status) {
  if (LIKELY(SUCCESS == status)) {
    if (0 != (THREAD_ADD_FETCH32(&sync->count, -updates))) {
      return;
    }
  } else {
    /* this is an error path so just use the atomic */
    sync->status = status;
    atomic_wmb();
    atomic_swap_32(&sync->count, 0);
  }
  WAIT_SYNC_SIGNAL(sync);
}
