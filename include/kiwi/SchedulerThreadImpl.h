#pragma once

#ifdef KIWI_SCHEDULER_USE_PTHREAD
#include "kiwi/SchedulerThreadImplPthreads.h"
#else
#error No Thread impl for this platform
#endif
