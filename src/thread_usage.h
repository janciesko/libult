#pragma once

/**
 * Check and see if the process is using multiple threads.
 *
 * @retval true If the process may have more than one thread.
 * @retval false If the process only has a single thread.
 *
 * The value that this function returns is influenced by:
 *
 * - how MPI_INIT or MPI_INIT_THREAD was invoked,
 * - what the final MPI thread level was determined to be,
 * - whether the OMPI or MPI libraries are multi-threaded
 *
 * MPI_INIT and MPI_INIT_THREAD (specifically, back-end OMPI startup
 * functions) invoke set_using_threads() to influence the value of
 * this function, depending on their situation. Some examples:
 *
 * - if MPI_INIT is invoked, and the ompi components in use are
 * single-threaded, this value will be false.
 *
 * - if MPI_INIT_THREAD is invoked with MPI_THREAD_MULTIPLE, we have
 * thread support, and the final thread level is determined to be
 * MPI_THREAD_MULTIPLE, this value will be true.
 *
 * - if the process is a single-threaded OMPI executable (e.g., mpicc),
 * this value will be false.
 *
 * Hence, this function will return false if there is guaranteed to
 * only be one thread in the process.  If there is even the
 * possibility that we may have multiple threads, true will be
 * returned.
 */
#define using_threads() uses_threads

/**
 * Set whether the process is using multiple threads or not.
 *
 * @param have Boolean indicating whether the process is using
 * multiple threads or not.
 *
 * @retval using_threads The new return value from
 * using_threads().
 *
 * This function is used to influence the return value of
 * using_threads().  If configure detected that we have thread
 * support, the return value of future invocations of
 * using_threads() will be the parameter's value.  If configure
 * detected that we have no thread support, then the return from
 * using_threads() will always be false.
 */
static inline bool set_using_threads(bool have) {
  uses_threads = have;
  return using_threads();
}

/**
 * Use an atomic operation for increment/decrement if using_threads()
 * indicates that threads are in use by the application or library.
 */

#define THREAD_DEFINE_ATOMIC_OP(type, name, operator, suffix)                  \
  static inline type thread_##name##_fetch_##suffix(atomic_##type *addr,       \
                                                    type delta) {              \
    if (UNLIKELY(using_threads())) {                                           \
      return atomic_##name##_fetch_##suffix(addr, delta);                      \
    }                                                                          \
                                                                               \
    *addr = *addr operator delta;                                              \
    return *addr;                                                              \
  }                                                                            \
                                                                               \
  static inline type thread_fetch_##name##_##suffix(atomic_##type *addr,       \
                                                    type delta) {              \
    if (UNLIKELY(using_threads())) {                                           \
      return atomic_fetch_##name##_##suffix(addr, delta);                      \
    }                                                                          \
                                                                               \
    type old = *addr;                                                          \
    *addr = old operator delta;                                                \
    return old;                                                                \
  }

#define THREAD_DEFINE_ATOMIC_COMPARE_EXCHANGE(type, addr_type, suffix)         \
  static inline bool thread_compare_exchange_strong_##suffix(                  \
      atomic_##addr_type *addr, type *compare, type value) {                   \
    if (UNLIKELY(using_threads())) {                                           \
      return atomic_compare_exchange_strong_##suffix(                          \
          addr, (addr_type *)compare, (addr_type)value);                       \
    }                                                                          \
                                                                               \
    if ((type)*addr == *compare) {                                             \
      ((type *)addr)[0] = value;                                               \
      return true;                                                             \
    }                                                                          \
                                                                               \
    *compare = ((type *)addr)[0];                                              \
                                                                               \
    return false;                                                              \
  }

#define THREAD_DEFINE_ATOMIC_SWAP(type, addr_type, suffix)                     \
  static inline type thread_swap_##suffix(atomic_##addr_type *ptr,             \
                                          type newvalue) {                     \
    if (using_threads()) {                                                     \
      return (type)atomic_swap_##suffix(ptr, (addr_type)newvalue);             \
    }                                                                          \
                                                                               \
    type old = ((type *)ptr)[0];                                               \
    ((type *)ptr)[0] = newvalue;                                               \
                                                                               \
    return old;                                                                \
  }

THREAD_DEFINE_ATOMIC_OP(int32_t, add, +, 32)
THREAD_DEFINE_ATOMIC_OP(size_t, add, +, size_t)
THREAD_DEFINE_ATOMIC_OP(int32_t, and, &, 32)
THREAD_DEFINE_ATOMIC_OP(int32_t, or, |, 32)
THREAD_DEFINE_ATOMIC_OP(int32_t, xor, ^, 32)
THREAD_DEFINE_ATOMIC_OP(int32_t, sub, -, 32)
THREAD_DEFINE_ATOMIC_OP(size_t, sub, -, size_t)

THREAD_DEFINE_ATOMIC_COMPARE_EXCHANGE(int32_t, int32_t, 32)
THREAD_DEFINE_ATOMIC_COMPARE_EXCHANGE(intptr_t, intptr_t, ptr)
THREAD_DEFINE_ATOMIC_SWAP(int32_t, int32_t, 32)
THREAD_DEFINE_ATOMIC_SWAP(intptr_t, intptr_t, ptr)

#define THREAD_ADD_FETCH32 thread_add_fetch_32
#define ATOMIC_ADD_FETCH32 thread_add_fetch_32

#define THREAD_AND_FETCH32 thread_and_fetch_32
#define ATOMIC_AND_FETCH32 thread_and_fetch_32

#define THREAD_OR_FETCH32 thread_or_fetch_32
#define ATOMIC_OR_FETCH32 thread_or_fetch_32

#define THREAD_XOR_FETCH32 thread_xor_fetch_32
#define ATOMIC_XOR_FETCH32 thread_xor_fetch_32

#define THREAD_ADD_FETCH_SIZE_T thread_add_fetch_size_t
#define ATOMIC_ADD_FETCH_SIZE_T thread_add_fetch_size_t

#define THREAD_SUB_FETCH_SIZE_T thread_sub_fetch_size_t
#define ATOMIC_SUB_FETCH_SIZE_T thread_sub_fetch_size_t

#define THREAD_FETCH_ADD32 thread_fetch_add_32
#define ATOMIC_FETCH_ADD32 thread_fetch_add_32

#define THREAD_FETCH_AND32 thread_fetch_and_32
#define ATOMIC_FETCH_AND32 thread_fetch_and_32

#define THREAD_FETCH_OR32 thread_fetch_or_32
#define ATOMIC_FETCH_OR32 thread_fetch_or_32

#define THREAD_FETCH_XOR32 thread_fetch_xor_32
#define ATOMIC_FETCH_XOR32 thread_fetch_xor_32

#define THREAD_FETCH_ADD_SIZE_T thread_fetch_add_size_t
#define ATOMIC_FETCH_ADD_SIZE_T thread_fetch_add_size_t

#define THREAD_FETCH_SUB_SIZE_T thread_fetch_sub_size_t
#define ATOMIC_FETCH_SUB_SIZE_T thread_fetch_sub_size_t

#define THREAD_COMPARE_EXCHANGE_STRONG_32 thread_compare_exchange_strong_32
#define ATOMIC_COMPARE_EXCHANGE_STRONG_32 thread_compare_exchange_strong_32

#define THREAD_COMPARE_EXCHANGE_STRONG_PTR(x, y, z)                            \
  thread_compare_exchange_strong_ptr((atomic_intptr_t *)x, (intptr_t *)y,      \
                                     (intptr_t)z)
#define ATOMIC_COMPARE_EXCHANGE_STRONG_PTR THREAD_COMPARE_EXCHANGE_STRONG_PTR

#define THREAD_SWAP_32 thread_swap_32
#define ATOMIC_SWAP_32 thread_swap_32

#define THREAD_SWAP_PTR(x, y) thread_swap_ptr((atomic_intptr_t *)x, (intptr_t)y)
#define ATOMIC_SWAP_PTR THREAD_SWAP_PTR

THREAD_DEFINE_ATOMIC_OP(int64_t, add, +, 64)
THREAD_DEFINE_ATOMIC_OP(int64_t, and, &, 64)
THREAD_DEFINE_ATOMIC_OP(int64_t, or, |, 64)
THREAD_DEFINE_ATOMIC_OP(int64_t, xor, ^, 64)
THREAD_DEFINE_ATOMIC_OP(int64_t, sub, -, 64)
THREAD_DEFINE_ATOMIC_COMPARE_EXCHANGE(int64_t, int64_t, 64)
THREAD_DEFINE_ATOMIC_SWAP(int64_t, int64_t, 64)

#define THREAD_ADD_FETCH64 thread_add_fetch_64
#define ATOMIC_ADD_FETCH64 thread_add_fetch_64

#define THREAD_AND_FETCH64 thread_and_fetch_64
#define ATOMIC_AND_FETCH64 thread_and_fetch_64

#define THREAD_OR_FETCH64 thread_or_fetch_64
#define ATOMIC_OR_FETCH64 thread_or_fetch_64

#define THREAD_XOR_FETCH64 thread_xor_fetch_64
#define ATOMIC_XOR_FETCH64 thread_xor_fetch_64

#define THREAD_FETCH_ADD64 thread_fetch_add_64
#define ATOMIC_FETCH_ADD64 thread_fetch_add_64

#define THREAD_FETCH_AND64 thread_fetch_and_64
#define ATOMIC_FETCH_AND64 thread_fetch_and_64

#define THREAD_FETCH_OR64 thread_fetch_or_64
#define ATOMIC_FETCH_OR64 thread_fetch_or_64

#define THREAD_FETCH_XOR64 thread_fetch_xor_64
#define ATOMIC_FETCH_XOR64 thread_fetch_xor_64

#define THREAD_COMPARE_EXCHANGE_STRONG_64 thread_compare_exchange_strong_64
#define ATOMIC_COMPARE_EXCHANGE_STRONG_64 thread_compare_exchange_strong_64

#define THREAD_SWAP_64 thread_swap_64
#define ATOMIC_SWAP_64 thread_swap_64

/* thread local storage */
#if C_HAVE__THREAD_LOCAL
#define thread_local _Thread_local
#define HAVE_THREAD_LOCAL 1

#elif C_HAVE___THREAD /* C_HAVE__THREAD_LOCAL */
#define thread_local __thread
#define HAVE_THREAD_LOCAL 1
#endif /* C_HAVE___THREAD */

#if !defined(HAVE_THREAD_LOCAL)
#define HAVE_THREAD_LOCAL 0
#endif /* !defined(HAVE_THREAD_LOCAL) */
