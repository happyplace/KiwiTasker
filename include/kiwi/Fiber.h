#pragma once

#include "kiwi/Config.h"
#include "kiwi/Context.h"
#include "kiwi/Job.h"

namespace kiwi
{
class Counter;

struct Fiber
{
    char m_stack[KiwiConfig::fiberStackSize];
    kiwi::Context m_context = {0};
    kiwi::Job m_job = {0};
    union
    {
        Counter* m_counter = nullptr;
        
        // used by fiber pool and counters
        Fiber* next_;
    };   
#ifdef KIWI_HAS_VALGRIND
    unsigned stack_id = 0;
#endif // KIWI_HAS_VALGRIND
};
}
