#pragma once

#ifndef NDEBUG
#include <setjmp.h>
#endif // NDEBUG

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

#ifndef NDEBUG
  char scratch[sizeof(jmp_buf)];
#endif // NDEBUG
};

}
