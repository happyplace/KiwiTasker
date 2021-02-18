#pragma once

#include <inttypes.h>

#include <queue>
#include <list>

#include "kiwi/SpinLock.h"
#include "kiwi/JobPriority.h"

namespace kiwi
{
struct Job;
class Counter;
struct FiberWorkerData;
struct Fiber;

class Scheduler
{
public:
    Scheduler();
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    void Init();

    // add a job to the queue, once this function returns it's safe to delete the job object because a copy
    // of the job structure is saved. If a counter is provided it will be incremented for the job added
    void AddJob(const Job& job, const JobPriority priority = JobPriority::Normal, Counter* counter = nullptr);

    void WaitForCounter(Counter* counter, uint64_t value = 0);

//private:
    struct PendingJob
    {
        Job* m_job = nullptr;
        JobPriority m_priority = JobPriority::Normal;
        Counter* m_counter = nullptr;
    };

    SpinLock m_pendingTasksLock;
    std::queue<PendingJob> m_pendingTasks;

    FiberWorkerData* m_fiberWorkerData = nullptr;

    std::vector<Fiber*> m_waitList;
    std::list<Fiber*> m_readyList;
};
}
