#pragma once

#include "kiwi/Job.h"
#include "kiwi/JobPriority.h"

namespace kiwi
{
class Counter;

struct PendingJob
{
    Job m_job;
    Counter* m_counter = nullptr;
};
}
