#pragma once

#include "kiwi/Context.h"
#include "kiwi/Job.h"

namespace kiwi
{
class Counter;

struct Fiber
{
    char m_stack[4096];
    kiwi::Context m_context;
    kiwi::Job m_job;
    union
    {
        kiwi::Counter* m_counter = nullptr;
        
        // used by fiber pool
        Fiber* next_;
    };    
};
}
