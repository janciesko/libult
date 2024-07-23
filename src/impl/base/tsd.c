#include "tsd.h"

static void _tracked_destructor(void *arg)
{
    tsd_list_item_t *tsd = NULL;
    tsd_tracked_key_t *key = NULL;
    if (NULL == arg) {
        return;
    }

    tsd = (tsd_list_item_t *) arg;
    key = tsd->tracked_key;

    mutex_lock(&key->mutex);
    list_remove_item(&key->tsd_list, &tsd->super);
    mutex_unlock(&key->mutex);

    if (NULL != key->user_destructor) {
        key->user_destructor(tsd->data);
    }
    OBJ_RELEASE(tsd);
}

void tsd_tracked_key_constructor(tsd_tracked_key_t *key)
{
    OBJ_CONSTRUCT(&key->mutex, mutex_t);
    OBJ_CONSTRUCT(&key->tsd_list, list_t);
    key->user_destructor = NULL;
    tsd_key_create(&key->key, _tracked_destructor);
}

void tsd_tracked_key_destructor(tsd_tracked_key_t *key)
{
    tsd_list_item_t *tsd, *next;

    tsd_key_delete(key->key);
    LIST_FOREACH_SAFE (tsd, next, &key->tsd_list, tsd_list_item_t) {
        list_remove_item(&key->tsd_list, &tsd->super);
        if (NULL != key->user_destructor) {
            key->user_destructor(tsd->data);
        }
        OBJ_RELEASE(tsd);
    }
    OBJ_DESTRUCT(&key->mutex);
    OBJ_DESTRUCT(&key->tsd_list);
}

int tsd_tracked_key_set(tsd_tracked_key_t *key, void *p)
{
    assert(NULL != key);

    tsd_list_item_t *tsd = NULL;
    tsd_get(key->key, (void **) &tsd);

    if (NULL == tsd) {
        tsd = OBJ_NEW(tsd_list_item_t);
        if (NULL == tsd) {
            return ERR_OUT_OF_RESOURCE;
        }

        mutex_lock(&key->mutex);
        list_append(&key->tsd_list, &tsd->super);
        mutex_unlock(&key->mutex);
    }

    tsd->data = p;
    tsd->tracked_key = key;

    return tsd_set(key->key, (void *) tsd);
}

void tsd_tracked_key_set_destructor(tsd_tracked_key_t *key,
                                         tsd_destructor_t destructor)
{
    assert(NULL != key);
    key->user_destructor = destructor;
}

OBJ_CLASS_INSTANCE(tsd_list_item_t, list_item_t, NULL, NULL);
OBJ_CLASS_INSTANCE(tsd_tracked_key_t, object_t, tsd_tracked_key_constructor,
                   tsd_tracked_key_destructor);
