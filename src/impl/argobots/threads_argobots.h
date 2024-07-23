#pragma once

#include "abt.h"

static inline void threads_argobots_ensure_init(void) {
  if (ABT_SUCCESS != ABT_initialized()) {
    ABT_init(0, 0);
  }
}
