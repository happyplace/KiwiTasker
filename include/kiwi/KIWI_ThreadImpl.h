#pragma once

#if defined(KIWI_USE_WINTHREAD_IMPL)
#include "kiwi/KIWI_ThreadImplWinThread.h"
#else
#error This platform has no thread implementation KiwiTasker
#endif
