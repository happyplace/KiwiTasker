#pragma once

#ifdef _WIN64
#include <emmintrin.h>
#endif // _WIN64

#include "kiwi/Config.h"

namespace kiwi
{
struct Context
{
#if defined(__linux__) && !defined(__ANDROID__) && defined(__x86_64__)
    void *rip, *rsp;
    void *rbx, *rbp, *r12, *r13, *r14, *r15;
    void *rdi, *rsi, *rdx;
#elif defined(_WIN64)
    void *rip, *rsp;
    void *rbx, *rbp, *r12, *r13, *r14, *r15, *rdi, *rsi;
    __m128i xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    void *rcx, *rdx, *r8;
#else
#error There is no context structure for this platform
#endif

#ifdef KIWI_HAS_VALGRIND
    unsigned stack_id = 0;
#endif // KIWI_HAS_VALGRIND
};
}
