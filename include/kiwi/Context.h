#pragma once

#include "kiwi/Config.h"

namespace kiwi
{
struct Context
{
#if defined(__linux__) && !defined(__ANDROID__) && defined(__x86_64__)
    void *rip, *rsp;
    void *rbx, *rbp, *r12, *r13, *r14, *r15;
    void *rdi, *rsi, *rdx;
#else
#error There is no context structure for this platform
#endif

#ifdef KIWI_HAS_VALGRIND
    unsigned stack_id = 0;
#endif // KIWI_HAS_VALGRIND
};
}
