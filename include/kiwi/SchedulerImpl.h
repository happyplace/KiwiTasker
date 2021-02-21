#pragma once

#if defined(__linux__) && !defined(__ANDROID__)
#include "SchedulerImpl_unix.h"
#else
#error "This platform does not have a Scheduler Implementation class"
#endif
