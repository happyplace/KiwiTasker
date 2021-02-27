#pragma once

#include "kiwi/Config.h"
#include "kiwi/Context.h"
#include "kiwi/Job.h"

class Counter;

namespace kiwi
{
struct Fiber
{
    char m_stack[KiwiConfig::fiberStackSize];
    kiwi::Context m_context = {0};
    kiwi::Context m_returnContext = {0};
    kiwi::Job m_job = {0};
    union
    {
        Counter* m_counter = nullptr;
        
        // used by fiber pool and counters
        Fiber* next_;
    };    
};
}
