#pragma once

#include <atomic>
#include <condition_variable>
#include <inttypes.h>
#include <mutex>

#include "kiwi/JobPriority.h"
#include "kiwi/Queue.h"
#include "kiwi/SpinLock.h"

namespace kiwi
{
class SchedulerImpl;
class Counter;
struct FiberWorkerStorage;
struct Job;
struct PendingJob;
class FiberPool;

class Scheduler
{
public:
    Scheduler();
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    // initializes memory and starts worker threads, this function should be called before calling 
    // any other functions on the Scheduler.
    // all the clean up for this function is handled in the decontructor 
    void Init();

    // add a job to the queue, once this function returns it's safe to delete the job object because a copy
    // of the job structure is saved. If a counter is provided it will be incremented for the job added
    void AddJob(const Job* job, const JobPriority priority = JobPriority::Normal, Counter* counter = nullptr);

    // add multiple jobs to the queue, once this function returns it's safe to delete the job objects because a copy
    // of the job structures is saved. If a counter is provided it will be incremented for each job added
    void AddJob(const Job* jobs, const uint32_t size, const JobPriority priority = JobPriority::Normal, Counter* counter = nullptr);

    void WaitForCounter(Counter* counter, uint64_t value = 0);    

    SchedulerImpl* GetImpl();

    FiberWorkerStorage* GetFiberWorkerStorage();

private:
    SchedulerImpl* m_impl = nullptr;
    FiberWorkerStorage* m_workerStorage = nullptr;

    SpinLock m_queueLock;    
    Queue<PendingJob> m_queueHigh;
    Queue<PendingJob> m_queueNormal;    
    Queue<PendingJob> m_queueLow;

    FiberPool* m_fiberPool = nullptr;

    std::condition_variable m_conditionVariable;
    std::mutex m_mutex;
    std::atomic_bool m_closeWorkers;
};
}
