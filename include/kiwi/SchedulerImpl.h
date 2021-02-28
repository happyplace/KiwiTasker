#pragma once

#include <inttypes.h>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "kiwi/Array.h"
#include "kiwi/Config.h"
#include "kiwi/Counter.h"
#include "kiwi/Fiber.h"
#include "kiwi/FiberPool.h"
#include "kiwi/Job.h"
#include "kiwi/JobPriority.h"
#include "kiwi/Queue.h"
#include "kiwi/SchedulerThreadImpl.h"
#include "kiwi/SpinLock.h"

namespace kiwi
{
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

struct PendingJob
{
    Job m_job = {0};
    Counter* m_counter = nullptr;
};

struct WaitingFiber
{
    int64_t m_targetValue = 0;
    Fiber* m_fiber = nullptr;
    Counter* m_counter = nullptr;
};

class SchedulerImpl
{
public:
    SchedulerImpl();
    ~SchedulerImpl();

    SchedulerImpl(const SchedulerImpl&) = delete;
    SchedulerImpl& operator=(const SchedulerImpl&) = delete;

    void Init(Scheduler* scheduler);
    void AddJobs(const Job* jobs, const uint32_t size, const JobPriority priority = JobPriority::Normal, Counter* counter = nullptr);
    void WaitForCounter(Counter* counter, int64_t value = 0); 

    bool GetNextAvailableFiber(Fiber** outFiber, bool& outResume);
    void ReturnFiber(Fiber* fiber);

    const SchedulerThreadImpl& GetThreadImpl() const { return m_threadImpl; }

#ifdef KIWI_SCHEDULER_ERROR_CHECKING
    // If there is any fiber waiting on this counter and the counter gets deleted this will force wake it up.
    // this is considered an error and will assert if extra error checking is enabled.
    // this should be called by counters when they're deleted
    void WakeAnyFibersWaitingOnCounter(Counter* counter);
#endif // KIWI_SCHEDULER_ERROR_CHECKING

private:
    SchedulerThreadImpl m_threadImpl;
    SchedulerWorkerStartParams m_threadStartParams;
    std::condition_variable m_workerConditionVariable;
    std::mutex m_workerMutex;
    std::atomic_bool m_workerExit = ATOMIC_VAR_INIT(false);
    bool m_initialized = false;
    
    SpinLock m_queueSpinLock;
    Queue<PendingJob> m_queueHigh;
    Queue<PendingJob> m_queueNormal;
    Queue<PendingJob> m_queueLow;
    Array<Fiber*> m_pendingFiber;

    SpinLock m_waitingFiberLock;
    Array<WaitingFiber> m_waitingFibers;

    FiberPool m_fiberPool;
};
}
