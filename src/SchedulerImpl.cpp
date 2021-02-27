#include "kiwi/SchedulerImpl.h"

#include <assert.h>

#include "kiwi/FiberWorkerStorage.h"
#include "kiwi/Scheduler.h"

using namespace kiwi;

SchedulerImpl::SchedulerImpl()
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

}

void SchedulerImpl::WaitForCounter(Counter* counter, uint64_t value /*= 0*/)
{

}
