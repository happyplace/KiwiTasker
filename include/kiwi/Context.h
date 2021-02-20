#pragma once

namespace kiwi
{
//
#if defined(__linux__) && !defined(__ANDROID__) && defined(__x86_64__)
struct Context
{
    void *rip, *rsp;
    void *rbx, *rbp, *r12, *r13, *r14, *r15;
    void *rdi, *rsi, *rdx;
};
#else
#error There is no context structure for this platform
#endif
}
