#pragma once

#include <inttypes.h>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "kiwi/JobPriority.h"
#include "kiwi/SchedulerThreadImpl.h"

namespace kiwi
{
class Counter;
struct Job;
class Scheduler;
class SchedulerImpl;

struct SchedulerWorkerStartParams
{
    Scheduler* m_scheduler = nullptr;
    SchedulerImpl* m_schedulerImpl = nullptr;
    std::condition_variable* m_workerConditionVariable = nullptr;
    std::mutex* m_workerMutex = nullptr;
    std::atomic_bool* m_workerExit = nullptr;
};

class SchedulerImpl
{
public:
    SchedulerImpl();
    ~SchedulerImpl();

    SchedulerImpl(const SchedulerImpl&) = delete;
    SchedulerImpl& operator=(const SchedulerImpl&) = delete;

    void Init(Scheduler* scheduler);
    void AddJob(const Job* jobs, const uint32_t size, const JobPriority priority = JobPriority::Normal, Counter* counter = nullptr);
    void WaitForCounter(Counter* counter, uint64_t value = 0); 

private:
    SchedulerThreadImpl m_threadImpl;
    SchedulerWorkerStartParams m_threadStartParams;
    std::condition_variable m_workerConditionVariable;
    std::mutex m_workerMutex;
    std::atomic_bool m_workerExit = ATOMIC_VAR_INIT(false);
    bool m_initialized = false;
};
}
