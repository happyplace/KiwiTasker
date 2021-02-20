#include "kiwi/Scheduler.h"

#include "kiwi/Counter.h"
#include "kiwi/FiberPool.h"
#include "kiwi/FiberWorkerStorage.h"
#include "kiwi/Job.h"
#include "kiwi/PendingJob.h"

#if defined(__linux__) && !defined(__ANDROID__)
#include "SchedulerImpl_unix.h"
#else
#error "This platform does not have a Scheduler Implementation class"
#endif

using namespace kiwi;

Scheduler::Scheduler()
    : m_queueHigh(KiwiConfig::schedulerQueueSize)
    , m_queueNormal(KiwiConfig::schedulerQueueSize)    
    , m_queueLow(KiwiConfig::schedulerQueueSize)
{
    m_impl = new SchedulerImpl();   
    m_fiberPool = new FiberPool(KiwiConfig::schedulerFiberPoolSize);
}

Scheduler::~Scheduler()
{
    // lock mutex so no thread can sleep between the close worker value change and the notify all
    m_mutex.lock();
    m_closeWorkers.store(true);
    m_mutex.unlock();

    m_conditionVariable.notify_all();

    // we can't call delete on impl until all the worker threads have been signaled that they
    // should shutdown and woken up
    delete m_impl;

    delete m_fiberPool;

    if (m_workerStorage != nullptr)
    {
        delete[] m_workerStorage;
    }
}

void* FiberWorkerThreadMain(void* arg)
{
    FiberWorkerStorage* storage = reinterpret_cast<FiberWorkerStorage*>(arg);

    while (!storage->m_closeWorker->load())
    {
        Fiber* fiber = nullptr;        
        if (storage->GetOrWaitForNextFiber(&fiber))
        {
            storage->m_fiberPool->ReturnFiber(fiber);
        }
    }

    return 0;
}

void Scheduler::Init()
{
    constexpr size_t threadNameBufferSize = 128;
    constexpr int threadNameMaxSize = 16;
    char threadNameBuffer[threadNameBufferSize];

    const int32_t cpuCount = m_impl->GetCPUCount();

    m_workerStorage = new FiberWorkerStorage[cpuCount];
    m_closeWorkers.store(false);

    for (int32_t i = 0; i < cpuCount; ++i)
    {
        FiberWorkerStorage* storage = &m_workerStorage[i];
        storage->m_scheduler = this;
        storage->m_conditionVariable = &m_conditionVariable;
        storage->m_mutex = &m_mutex;
        storage->m_closeWorker = &m_closeWorkers;
        storage->m_queueLock = &m_queueLock;    
        storage->m_queueHigh = &m_queueHigh;
        storage->m_queueNormal = &m_queueNormal;    
        storage->m_queueLow = &m_queueLow;
        storage->m_fiberPool = m_fiberPool;

        int result = snprintf(threadNameBuffer, threadNameBufferSize, "Kiwi Worker %i", i);
        // if the resulting thread name is going to be larger than the max thread name size, truncate the name
        if (result > threadNameMaxSize)
        {
            threadNameBuffer[threadNameMaxSize - 1] = '\0';
        }
        m_impl->CreateThread(threadNameBuffer, i, FiberWorkerThreadMain, storage); 
    }
}

void Scheduler::AddJob(const Job* job, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    AddJob(job, 1, priority, counter);
}

void Scheduler::AddJob(const Job* jobs, const uint32_t size, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    for (uint32_t i = 0; i < size; ++i)
    {
        PendingJob pendingJob;
        pendingJob.m_job = jobs[i]; 
        pendingJob.m_counter = counter;

        m_queueLock.Lock();
        switch (priority)
        {
        case JobPriority::High:
            m_queueHigh.Push(pendingJob);
            break;
        case JobPriority::Normal:
            m_queueNormal.Push(pendingJob);
            break;
        case JobPriority::Low:
            m_queueLow.Push(pendingJob);
            break;
        default:
            assert(!"unsupported job priority");
            break;    
        }
        m_queueLock.Unlock();
    }
}
