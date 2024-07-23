#define once

/**
 * @file:
 *
 * Mutual exclusion functions.
 *
 * Functions for locking of critical sections.
 */

/**
 * Opaque mutex object
 */

typedef struct mutex_t mutex_t;
typedef struct mutex_t recursive_mutex_t;

struct mutex_t {
  object_t super;
  thread_internal_mutex_t m_lock;
#if ENABLE_DEBUG
  int m_lock_debug;
  const char *m_lock_file;
  int m_lock_line;
#endif
  atomic_lock_t m_lock_atomic;
};

DECLSPEC OBJ_CLASS_DECLARATION(mutex_t);
DECLSPEC OBJ_CLASS_DECLARATION(recursive_mutex_t);

#if ENABLE_DEBUG
#define MUTEX_STATIC_INIT                                                      \
  {                                                                            \
    .super = OBJ_STATIC_INIT(mutex_t),                                         \
    .m_lock = THREAD_INTERNAL_MUTEX_INITIALIZER, .m_lock_debug = 0,            \
    .m_lock_file = NULL, .m_lock_line = 0, .m_lock_atomic = ATOMIC_LOCK_INIT,  \
  }
#else
#define MUTEX_STATIC_INIT                                                      \
  {                                                                            \
    .super = OBJ_STATIC_INIT(mutex_t),                                         \
    .m_lock = THREAD_INTERNAL_MUTEX_INITIALIZER,                               \
    .m_lock_atomic = ATOMIC_LOCK_INIT,                                         \
  }
#endif

#if defined(THREAD_INTERNAL_RECURSIVE_MUTEX_INITIALIZER)
#if ENABLE_DEBUG
#define RECURSIVE_MUTEX_STATIC_INIT                                            \
  {                                                                            \
    .super = OBJ_STATIC_INIT(mutex_t),                                         \
    .m_lock = THREAD_INTERNAL_RECURSIVE_MUTEX_INITIALIZER, .m_lock_debug = 0,  \
    .m_lock_file = NULL, .m_lock_line = 0, .m_lock_atomic = ATOMIC_LOCK_INIT,  \
  }
#else
#define RECURSIVE_MUTEX_STATIC_INIT                                            \
  {                                                                            \
    .super = OBJ_STATIC_INIT(mutex_t),                                         \
    .m_lock = THREAD_INTERNAL_RECURSIVE_MUTEX_INITIALIZER,                     \
    .m_lock_atomic = ATOMIC_LOCK_INIT,                                         \
  }
#endif
#endif /* THREAD_INTERNAL_RECURSIVE_MUTEX_INITIALIZER */

/**
 * Try to acquire a mutex.
 *
 * @param mutex         Address of the mutex.
 * @return              0 if the mutex was acquired, 1 otherwise.
 */
static inline int mutex_trylock(mutex_t *mutex) {
  return thread_internal_mutex_trylock(&mutex->m_lock);
}

/**
 * Acquire a mutex.
 *
 * @param mutex         Address of the mutex.
 */
static inline void mutex_lock(mutex_t *mutex) {
  thread_internal_mutex_lock(&mutex->m_lock);
}

/**
 * Release a mutex.
 *
 * @param mutex         Address of the mutex.
 */
static inline void mutex_unlock(mutex_t *mutex) {
  thread_internal_mutex_unlock(&mutex->m_lock);
}

/**
 * Try to acquire a mutex using atomic operations.
 *
 * @param mutex         Address of the mutex.
 * @return              0 if the mutex was acquired, 1 otherwise.
 */
static inline int mutex_atomic_trylock(mutex_t *mutex) {
  return atomic_trylock(&mutex->m_lock_atomic);
}

/**
 * Acquire a mutex using atomic operations.
 *
 * @param mutex         Address of the mutex.
 */
static inline void mutex_atomic_lock(mutex_t *mutex) {
  atomic_lock(&mutex->m_lock_atomic);
}

/**
 * Release a mutex using atomic operations.
 *
 * @param mutex         Address of the mutex.
 */
static inline void mutex_atomic_unlock(mutex_t *mutex) {
  atomic_unlock(&mutex->m_lock_atomic);
}

/**
 * Lock a mutex if using_threads() says that multiple threads may
 * be active in the process.
 *
 * @param mutex Pointer to a mutex_t to lock.
 *
 * If there is a possibility that multiple threads are running in the
 * process (as determined by using_threads()), this function will
 * block waiting to lock the mutex.
 *
 * If there is no possibility that multiple threads are running in the
 * process, return immediately.
 */
#define THREAD_LOCK(mutex)                                                     \
  do {                                                                         \
    if (UNLIKELY(using_threads())) {                                           \
      mutex_lock(mutex);                                                       \
    }                                                                          \
  } while (0)

/**
 * Try to lock a mutex if using_threads() says that multiple
 * threads may be active in the process.
 *
 * @param mutex Pointer to a mutex_t to trylock
 *
 * If there is a possibility that multiple threads are running in the
 * process (as determined by using_threads()), this function will
 * trylock the mutex.
 *
 * If there is no possibility that multiple threads are running in the
 * process, return immediately without modifying the mutex.
 *
 * Returns 0 if mutex was locked, non-zero otherwise.
 */
#define THREAD_TRYLOCK(mutex)                                                  \
  (UNLIKELY(using_threads()) ? mutex_trylock(mutex) : 0)

/**
 * Unlock a mutex if using_threads() says that multiple threads
 * may be active in the process.
 *
 * @param mutex Pointer to a mutex_t to unlock.
 *
 * If there is a possibility that multiple threads are running in the
 * process (as determined by using_threads()), this function will
 * unlock the mutex.
 *
 * If there is no possibility that multiple threads are running in the
 * process, return immediately without modifying the mutex.
 */
#define THREAD_UNLOCK(mutex)                                                   \
  do {                                                                         \
    if (UNLIKELY(using_threads())) {                                           \
      mutex_unlock(mutex);                                                     \
    }                                                                          \
  } while (0)

/**
 * Lock a mutex if using_threads() says that multiple threads may
 * be active in the process for the duration of the specified action.
 *
 * @param mutex    Pointer to a mutex_t to lock.
 * @param action   A scope over which the lock is held.
 *
 * If there is a possibility that multiple threads are running in the
 * process (as determined by using_threads()), this function will
 * acquire the lock before invoking the specified action and release
 * it on return.
 *
 * If there is no possibility that multiple threads are running in the
 * process, invoke the action without acquiring the lock.
 */
#define THREAD_SCOPED_LOCK(mutex, action)                                      \
  do {                                                                         \
    if (UNLIKELY(using_threads())) {                                           \
      mutex_lock(mutex);                                                       \
      action;                                                                  \
      mutex_unlock(mutex);                                                     \
    } else {                                                                   \
      action;                                                                  \
    }                                                                          \
  } while (0)

typedef thread_internal_cond_t cond_t;
#define CONDITION_STATIC_INIT THREAD_INTERNAL_COND_INITIALIZER
int cond_init(cond_t *cond);
int cond_wait(cond_t *cond, mutex_t *lock);
int cond_broadcast(cond_t *cond);
int cond_signal(cond_t *cond);
int cond_destroy(cond_t *cond);
