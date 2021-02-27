#pragma once

#include <inttypes.h>

#include "kiwi/JobPriority.h"

namespace kiwi
{
class Counter;
struct Job;

class SchedulerImpl
{
public:
    SchedulerImpl();
    ~SchedulerImpl();

    SchedulerImpl(const SchedulerImpl&) = delete;
    SchedulerImpl& operator=(const SchedulerImpl&) = delete;

    void Init();
    void AddJob(const Job* jobs, const uint32_t size, const JobPriority priority = JobPriority::Normal, Counter* counter = nullptr);
    void WaitForCounter(Counter* counter, uint64_t value = 0);    
};
}
