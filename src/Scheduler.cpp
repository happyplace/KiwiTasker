#include "kiwi/Scheduler.h"

#include "kiwi/Counter.h"
#include "kiwi/Job.h"
#include "kiwi/FiberWorkerStorage.h"

#if defined(__linux__) && !defined(__ANDROID__)
#include "SchedulerImpl_unix.h"
#else
#error "This platform does not have a Scheduler Implementation class"
#endif

using namespace kiwi;

Scheduler::Scheduler()
{
    m_impl = new SchedulerImpl();
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
            
        }
    }

    return 0;
}

void Scheduler::Init()
{
    constexpr size_t threadNameBufferSize = 16;
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

        snprintf(threadNameBuffer, threadNameBufferSize, "Kiwi Worker %i", i);
        m_impl->CreateThread(threadNameBuffer, i, FiberWorkerThreadMain, storage); 
    }
}

void Scheduler::AddJob(const Job& job, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    AddJob(&job, 1, priority, counter);
}

void Scheduler::AddJob(const Job* jobs, const uint32_t size, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    // PendingJob pendingJob;
    // pendingJob.m_job = new Job(job);
    // pendingJob.m_priority = priority;
    // pendingJob.m_counter = counter;

    // if (counter)
    // {
    //     counter->Increment();
    // }

    // m_pendingTasksLock.Lock();
    // m_pendingTasks.push(std::move(pendingJob));
    // m_pendingTasksLock.Unlock();
}
