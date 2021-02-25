#pragma once

#include "kiwi/Config.h"
#include "kiwi/Context.h"
#include "kiwi/Job.h"

namespace kiwi
{
class Counter;

struct Fiber
{
    char* m_stack[KiwiConfig::fiberSmallStackSize];
    kiwi::Context m_context = {0};
    kiwi::Job m_job = {0};
    union
    {
        kiwi::Counter* m_counter = nullptr;
        
        // used by fiber pool
        Fiber* next_;
    };    
};
}
