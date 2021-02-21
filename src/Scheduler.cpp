#include "kiwi/Scheduler.h"

#include "kiwi/Counter.h"
#include "kiwi/FiberPool.h"
#include "kiwi/FiberWorkerStorage.h"
#include "kiwi/Job.h"
#include "kiwi/PendingJob.h"
#include "kiwi/FiberWorkerStorage.h"
#include "kiwi/FiberWorkerThreadMain.h"
#include "kiwi/SchedulerImpl.h"

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
    {
        std::unique_lock<std::mutex> lck(m_mutex);
        m_closeWorkers.store(true);
    }

    m_conditionVariable.notify_all();

    // this function will block until all of the worker threads have been confirmed to be shutdown
    m_impl->ShutdownWorkerThreads();

    delete m_impl;
    m_impl = nullptr;

    delete m_fiberPool;
    m_fiberPool = nullptr;

    if (m_workerStorage != nullptr)
    {
        delete[] m_workerStorage;
        m_workerStorage = nullptr;
    }
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
        m_impl->CreateThread(threadNameBuffer, i, kiwi::FiberWorkerThreadMain, this); 
    }
}

SchedulerImpl* Scheduler::GetImpl()
{
    return m_impl;
}

FiberWorkerStorage* Scheduler::GetFiberWorkerStorage()
{
    return &m_workerStorage[m_impl->GetWorkerThreadIndex()];
}

void Scheduler::AddJob(const Job* job, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    AddJob(job, 1, priority, counter);
}

void Scheduler::AddJob(const Job* jobs, const uint32_t size, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    m_queueLock.Lock();

    bool queuesEmpty = m_queueHigh.IsEmpty() && m_queueNormal.IsEmpty() && m_queueLow.IsEmpty();

    for (uint32_t i = 0; i < size; ++i)
    {
        PendingJob pendingJob;
        pendingJob.m_job = jobs[i]; 
        pendingJob.m_counter = counter;                
        
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
    }

    m_queueLock.Unlock();

    if (queuesEmpty)
    {
        if (size > 0)
        {
            m_conditionVariable.notify_one();
        }
        else
        {
            m_conditionVariable.notify_all();
        }
    }
}
