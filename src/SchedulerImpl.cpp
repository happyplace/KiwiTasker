#include "kiwi/SchedulerImpl.h"

#include <assert.h>

#include "kiwi/Config.h"
#include "kiwi/FiberWorkerStorage.h"
#include "kiwi/Scheduler.h"

using namespace kiwi;

SchedulerImpl::SchedulerImpl()
    : m_queueHigh(KiwiConfig::schedulerQueueSize)
    , m_queueNormal(KiwiConfig::schedulerQueueSize)
    , m_queueLow(KiwiConfig::schedulerQueueSize)
    , m_pendingFiber(KiwiConfig::schdulerPendingFiberArraySize)
    , m_fiberPool(KiwiConfig::schedulerFiberPoolSize)
{
}

SchedulerImpl::~SchedulerImpl()
{
    if (m_initialized)
    {
        m_workerExit.store(true);
        m_workerConditionVariable.notify_all();
        m_threadImpl.JoinWorkerThreads();

        kiwi::DestroyFiberWorkerStorage();
    }
}

void* SchedulerWorkerThread(void* arg)
{
    SchedulerWorkerStartParams* params = reinterpret_cast<SchedulerWorkerStartParams*>(arg);
    (void)params;
    return NULL;
}

void SchedulerImpl::Init(Scheduler* scheduler)
{
    assert(!m_initialized && "Init() was called more than once");
    m_initialized = true;

    constexpr size_t threadNameBufferSize = 128;
    constexpr int threadNameMaxSize = 16;
    char threadNameBuffer[threadNameBufferSize];

    m_threadStartParams.m_scheduler = scheduler;
    m_threadStartParams.m_schedulerImpl = this;
    m_threadStartParams.m_workerConditionVariable = &m_workerConditionVariable;
    m_threadStartParams.m_workerExit = &m_workerExit;
    m_threadStartParams.m_workerMutex = &m_workerMutex;

    const int32_t cpuCount = m_threadImpl.GetCpuCount();

    kiwi::CreateFiberWorkerStorage(cpuCount);

    for (int32_t i = 0; i < cpuCount; ++i)
    {
        int result = snprintf(threadNameBuffer, threadNameBufferSize, "Kiwi Worker %i", i);
        // if the resulting thread name is going to be larger than the max thread name size, truncate the name
        if (result > threadNameMaxSize)
        {
            threadNameBuffer[threadNameMaxSize - 1] = '\0';
        }
        m_threadImpl.CreateThread(threadNameBuffer, i, SchedulerWorkerThread, &m_threadStartParams);
    }
}

void SchedulerImpl::AddJob(const Job* jobs, const uint32_t size, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    m_queueSpinLock.Lock();
    
    Queue<PendingJob>* queue = nullptr;
    switch (priority)
    {
        case JobPriority::High:
            queue = &m_queueHigh;
            break;
        case JobPriority::Normal:
            queue = &m_queueNormal;
            break;
        case JobPriority::Low:
            queue = &m_queueLow;
            break;
        default:
#ifdef KIWI_SCHEDULER_ERROR_CHECKING
            assert(!"Job priority does not exist, Normal priority will be used.");
#endif // KIWI_SCHEDULER_ERROR_CHECKING
            queue = &m_queueNormal;
            break;
    }

    PendingJob pendingJob;
    pendingJob.m_counter = counter;
    for (uint32_t i = 0; i < size; ++i)
    {
        pendingJob.m_job = jobs[i];
        queue->Push(pendingJob);
    }

    m_queueSpinLock.Unlock();
}

Fiber* SchedulerImpl::GetNextAvailableFiber()
{
    m_queueSpinLock.Lock();

    if (!m_queueHigh.IsEmpty())
    {
        PendingJob pendingJob;
        bool result = m_queueHigh.TryGetAndPopFront(&pendingJob);

#ifdef KIWI_SCHEDULER_ERROR_CHECKING
        assert(result && "Did someone change this list when we had the lock on it?");
#endif // KIWI_SCHEDULER_ERROR_CHECKING

        m_queueSpinLock.Unlock();
        
        Fiber* fiber = m_fiberPool.GetFiber();
        fiber->m_job = pendingJob.m_job;
        fiber->m_counter = pendingJob.m_counter;

        return fiber;
    }

    if (!m_pendingFiber.IsEmpty())
    {
        Fiber* fiber = m_pendingFiber[0];
        m_pendingFiber.Remove(0);

        m_queueSpinLock.Unlock();

        return fiber;
    }

    PendingJob pendingJob;
    bool gotJob = m_queueNormal.TryGetAndPopFront(&pendingJob);
    gotJob = !gotJob ? m_queueLow.TryGetAndPopFront(&pendingJob) : gotJob;
    
    m_queueSpinLock.Unlock();

    if (gotJob)
    {
        Fiber* fiber = m_fiberPool.GetFiber();
        fiber->m_job = pendingJob.m_job;
        fiber->m_counter = pendingJob.m_counter;

        return fiber;
    }

    return nullptr;
}

void SchedulerImpl::WaitForCounter(Counter* counter, uint64_t value /*= 0*/)
{

}
