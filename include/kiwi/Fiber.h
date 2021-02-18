#pragma once

#include "Context.h"

namespace kiwi
{
struct Job;

struct Fiber
{
    char m_stack[4096];
    kiwi::Context m_context;
    kiwi::Job* m_job = nullptr;
    kiwi::Counter* m_counter = nullptr;
};
}
