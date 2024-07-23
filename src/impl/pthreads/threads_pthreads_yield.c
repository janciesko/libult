
#include <time.h>
#ifdef HAVE_SCHED_H
#    include <sched.h>
#endif

#include "threads_pthreads.h"
#include "thread.h"

static void thread_pthreads_yield_sched_yield(void);
static void thread_pthreads_yield_nanosleep(void);

typedef enum {
    PTHREADS_YIELD_SCHED_YIELD = 0,
    PTHREADS_YIELD_NANOSLEEP
} threads_pthreads_yield_strategy_t;

static mca_base_var_enum_value_t yield_strategy_values[] = {{PTHREADS_YIELD_SCHED_YIELD,
                                                             "sched_yield"},
                                                            {PTHREADS_YIELD_NANOSLEEP,
                                                             "nanosleep"},
                                                            {0, NULL}};

/* Number of nanoseconds to nanosleep, if enabled */
static uint64_t yield_nsleep_nanosecs;
/* The time to nanosleep, if enabled */
static struct timespec yield_nsleep_time = {.tv_sec = 0, .tv_nsec = 1};
static threads_pthreads_yield_strategy_t yield_strategy = PTHREADS_YIELD_SCHED_YIELD;

threads_pthreads_yield_fn_t *threads_pthreads_yield_fn
    = &thread_pthreads_yield_sched_yield;

int threads_pthreads_yield_init(const mca_base_component_t *component)
{
    mca_base_var_enum_t *yield_strategy_enumerator;
    mca_base_var_enum_create("pthread_yield_strategies", yield_strategy_values,
                             &yield_strategy_enumerator);

    (void) mca_base_component_var_register(component, "yield_strategy",
                                           "Pthread yield strategy to use", MCA_BASE_VAR_TYPE_INT,
                                           yield_strategy_enumerator, 0, 0, INFO_LVL_3,
                                           MCA_BASE_VAR_SCOPE_LOCAL, &yield_strategy);
    switch (yield_strategy) {
    case PTHREADS_YIELD_NANOSLEEP:
        threads_pthreads_yield_fn = &thread_pthreads_yield_nanosleep;
        break;
    default:
        /* use initial value */
        break;
    }

    OBJ_RELEASE(yield_strategy_enumerator);

    yield_nsleep_nanosecs = (yield_nsleep_time.tv_sec * 1E9) + yield_nsleep_time.tv_nsec;
    (void) mca_base_component_var_register(
        component, "nanosleep_time",
        "Number of nanoseconds to sleep when using nanosleep as the pthread yield strategy",
        MCA_BASE_VAR_TYPE_UINT64_T, NULL, 0, 0, INFO_LVL_3, MCA_BASE_VAR_SCOPE_LOCAL,
        &yield_nsleep_nanosecs);
    yield_nsleep_time.tv_sec = yield_nsleep_nanosecs / 1E9;
    yield_nsleep_time.tv_nsec = yield_nsleep_nanosecs - (uint64_t)(yield_nsleep_time.tv_sec * 1E9);

    return SUCCESS;
}

void thread_pthreads_yield_sched_yield(void)
{
#ifdef HAVE_SCHED_H
    sched_yield();
#endif
}

void thread_pthreads_yield_nanosleep(void)
{
    nanosleep(&yield_nsleep_time, NULL);
}
