#pragma once

#if defined(KIWI_SCHEDULER_USE_PTHREAD)
#include "kiwi/SchedulerThreadImplPthreads.h"
#elif defined(KIWI_SCHEDULER_USE_WINTHREADS)
#include "kiwi/SchedulerThreadImplWinThreads.h"
#else
#error No Thread impl for this platform
#endif
