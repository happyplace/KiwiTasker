#pragma once

#include "Context.h"

namespace kiwi
{
struct Job;
class Counter;

struct Fiber
{
    char m_stack[4096];
    kiwi::Context m_context;
    kiwi::Job* m_job = nullptr;
    union
    {
        kiwi::Counter* m_counter = nullptr;
        
        // used by fiber pool
        Fiber* next_;
    };    
};
}
