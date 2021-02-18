#pragma once

namespace kiwi
{
struct Context
{
    void *rip, *rsp;
    void *rbx, *rbp, *r12, *r13, *r14, *r15;
    void *rdi, *rsi, *rdx;
};
}
