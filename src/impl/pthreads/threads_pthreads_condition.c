#include "condition.h"

static void condition_construct(condition_t *c)
{
    c->c_waiting = 0;
    c->c_signaled = 0;
}

static void condition_destruct(condition_t *c)
{
}

OBJ_CLASS_INSTANCE(condition_t, object_t, condition_construct,
                   condition_destruct);
