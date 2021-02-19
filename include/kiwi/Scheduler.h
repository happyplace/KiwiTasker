#pragma once

#include <atomic>
#include <condition_variable>
#include <inttypes.h>
#include <mutex>

#include "kiwi/JobPriority.h"

namespace kiwi
{
class SchedulerImpl;
class Counter;
struct FiberWorkerStorage;
struct Job;

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

    // add multiple jobs to the queue, once this function returns it's safe to delete the job objects because a copy
    // of the job structures is saved. If a counter is provided it will be incremented for each job added
    void AddJob(const Job* jobs, const uint32_t size, const JobPriority priority = JobPriority::Normal, Counter* counter = nullptr);

    void WaitForCounter(Counter* counter, uint64_t value = 0);    

    // struct PendingJob
    // {
    //     Job* m_job = nullptr;
    //     JobPriority m_priority = JobPriority::Normal;
    //     Counter* m_counter = nullptr;
    // };

    // SpinLock m_pendingTasksLock;
    // std::queue<PendingJob> m_pendingTasks;

    // FiberWorkerData* m_fiberWorkerData = nullptr;

    // std::vector<Fiber*> m_waitList;
    // std::list<Fiber*> m_readyList;

private:
    SchedulerImpl* m_impl = nullptr;
    FiberWorkerStorage* m_workerStorage = nullptr;

    std::condition_variable m_conditionVariable;
    std::mutex m_mutex;
    std::atomic_bool m_closeWorkers;
};
}
