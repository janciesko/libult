
#pragma once

#include "condition.h"
#include "mutex.h"

typedef void *(*thread_fn_t)(object_t *);

#define THREAD_CANCELLED ((void *)1);

#include MCA_threads_base_include_HEADER

struct thread_t {
  object_t super;
  thread_fn_t t_run;
  void *t_arg;
  pthread_t t_handle;
};

typedef struct thread_t thread_t;

OBJ_CLASS_DECLARATION(thread_t);

#if ENABLE_DEBUG
DECLSPEC extern bool debug_threads;
#endif

DECLSPEC OBJ_CLASS_DECLARATION(thread_t);

#if ENABLE_DEBUG
#define ACQUIRE_THREAD(lck, cnd, act)                                          \
  do {                                                                         \
    THREAD_LOCK((lck));                                                        \
    if (debug_threads) {                                                       \
      output(0, "Waiting for thread %s:%d", __FILE__, __LINE__);               \
    }                                                                          \
    while (*(act)) {                                                           \
      condition_wait((cnd), (lck));                                            \
    }                                                                          \
    if (debug_threads) {                                                       \
      output(0, "Thread obtained %s:%d", __FILE__, __LINE__);                  \
    }                                                                          \
    *(act) = true;                                                             \
  } while (0);
#else
#define ACQUIRE_THREAD(lck, cnd, act)                                          \
  do {                                                                         \
    THREAD_LOCK((lck));                                                        \
    while (*(act)) {                                                           \
      condition_wait((cnd), (lck));                                            \
    }                                                                          \
    *(act) = true;                                                             \
  } while (0);
#endif

#if ENABLE_DEBUG
#define RELEASE_THREAD(lck, cnd, act)                                          \
  do {                                                                         \
    if (debug_threads) {                                                       \
      output(0, "Releasing thread %s:%d", __FILE__, __LINE__);                 \
    }                                                                          \
    *(act) = false;                                                            \
    condition_broadcast((cnd));                                                \
    THREAD_UNLOCK((lck));                                                      \
  } while (0);
#else
#define RELEASE_THREAD(lck, cnd, act)                                          \
  do {                                                                         \
    *(act) = false;                                                            \
    condition_broadcast((cnd));                                                \
    THREAD_UNLOCK((lck));                                                      \
  } while (0);
#endif

#define WAKEUP_THREAD(cnd, act)                                                \
  do {                                                                         \
    *(act) = false;                                                            \
    condition_broadcast((cnd));                                                \
  } while (0);

/* provide a macro for forward-proofing the shifting
 * of objects between libevent threads - at some point, we
 * may revamp that threading model */

/* post an object to another thread - for now, we
 * only have a memory barrier */
#define POST_OBJECT(o) atomic_wmb()

/* acquire an object from another thread - for now,
 * we only have a memory barrier */
#define ACQUIRE_OBJECT(o) atomic_rmb()

int thread_start(thread_t *);
int thread_join(thread_t *, void **thread_return);
bool thread_self_compare(thread_t *);
thread_t *thread_get_self(void);
void thread_kill(thread_t *, int sig);
void thread_set_main(void);
static inline void thread_yield(void);
