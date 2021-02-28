#pragma once

#if defined(__linux__) && !defined(__ANDROID__) && defined(__x86_64__)
#include "kiwi/ContextFunctionsLinux.h"
#else
#error no context functions for this platform and/or cpu architecture
#endif
