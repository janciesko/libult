#include "wait_sync.h"

static mutex_t wait_sync_lock = MUTEX_STATIC_INIT;
ompi_wait_sync_t *threads_base_wait_sync_list = NULL; /* not static for inline "wait_sync_st" */

void threads_base_wait_sync_global_wakeup_st(int status)
{
    ompi_wait_sync_t *sync;
    for (sync = threads_base_wait_sync_list; sync != NULL; sync = sync->next) {
        wait_sync_update(sync, 0, status);
    }
}

void threads_base_wait_sync_global_wakeup_mt(int status)
{
    ompi_wait_sync_t *sync;
    mutex_lock(&wait_sync_lock);
    for (sync = threads_base_wait_sync_list; sync != NULL; sync = sync->next) {
        /* sync_update is going to  take the sync->lock from within
         * the wait_sync_lock. Thread lightly here: Idealy we should
         * find a way to not take a lock in a lock as this is deadlock prone,
         * but as of today we are the only place doing this so it is safe.
         */
        wait_sync_update(sync, 0, status);
        if (sync->next == threads_base_wait_sync_list) {
            break; /* special case for rings */
        }
    }
    mutex_unlock(&wait_sync_lock);
}

static atomic_int32_t num_thread_in_progress = 0;

#define WAIT_SYNC_PASS_OWNERSHIP(who)                        \
    do {                                                     \
        thread_internal_mutex_lock(&(who)->lock);       \
        thread_internal_cond_signal(&(who)->condition); \
        thread_internal_mutex_unlock(&(who)->lock);     \
    } while (0)

int ompi_sync_wait_mt(ompi_wait_sync_t *sync)
{
    /* Don't stop if the waiting synchronization is completed. We avoid the
     * race condition around the release of the synchronization using the
     * signaling field.
     */
    if (sync->count <= 0) {
        return (0 == sync->status) ? SUCCESS : ERROR;
    }

    /* lock so nobody can signal us during the list updating */
    thread_internal_mutex_lock(&sync->lock);

    /* Now that we hold the lock make sure another thread has not already
     * call cond_signal.
     */
    if (sync->count <= 0) {
        thread_internal_mutex_unlock(&sync->lock);
        return (0 == sync->status) ? SUCCESS : ERROR;
    }

    /* Insert sync on the list of pending synchronization constructs */
    THREAD_LOCK(&wait_sync_lock);
    if (NULL == threads_base_wait_sync_list) {
        sync->next = sync->prev = sync;
        threads_base_wait_sync_list = sync;
    } else {
        sync->prev = threads_base_wait_sync_list->prev;
        sync->prev->next = sync;
        sync->next = threads_base_wait_sync_list;
        threads_base_wait_sync_list->prev = sync;
    }
    THREAD_UNLOCK(&wait_sync_lock);

    /**
     * If we are not responsible for progressing, go silent until something
     * worth noticing happen:
     *  - this thread has been promoted to take care of the progress
     *  - our sync has been triggered.
     */
check_status:
    if (sync != threads_base_wait_sync_list && num_thread_in_progress >= max_thread_in_progress) {
        thread_internal_cond_wait(&sync->condition, &sync->lock);

        /**
         * At this point either the sync was completed in which case
         * we should remove it from the wait list, or/and I was
         * promoted as the progress manager.
         */

        if (sync->count <= 0) { /* Completed? */
            thread_internal_mutex_unlock(&sync->lock);
            goto i_am_done;
        }
        /* either promoted, or spurious wakeup ! */
        goto check_status;
    }
    thread_internal_mutex_unlock(&sync->lock);

    THREAD_ADD_FETCH32(&num_thread_in_progress, 1);
    while (sync->count > 0) { /* progress till completion */
        /* don't progress with the sync lock locked or you'll deadlock */
        progress();
    }
    THREAD_ADD_FETCH32(&num_thread_in_progress, -1);

i_am_done:
    /* My sync is now complete. Trim the list: remove self, wake next */
    THREAD_LOCK(&wait_sync_lock);
    sync->prev->next = sync->next;
    sync->next->prev = sync->prev;
    /* In case I am the progress manager, pass the duties on */
    if (sync == threads_base_wait_sync_list) {
        threads_base_wait_sync_list = (sync == sync->next) ? NULL : sync->next;
        if (NULL != threads_base_wait_sync_list) {
            WAIT_SYNC_PASS_OWNERSHIP(threads_base_wait_sync_list);
        }
    }
    THREAD_UNLOCK(&wait_sync_lock);

    return (0 == sync->status) ? SUCCESS : ERROR;
}
