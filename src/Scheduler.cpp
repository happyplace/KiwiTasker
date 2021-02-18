#include "kiwi/Scheduler.h"

#include "Scheduler_internal.h"

#include "kiwi/Counter.h"
#include "kiwi/Job.h"
#include "kiwi/FiberWorkerData.h"
#include "kiwi/Fiber.h"

using namespace kiwi;

Scheduler::Scheduler()
{
}

Scheduler::~Scheduler()
{
    if (m_fiberWorkerData != nullptr)
    {
        delete[] m_fiberWorkerData;
    }
}

void Scheduler::Init()
{
    // TODO:
}

void Scheduler::AddJob(const Job& job, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    PendingJob pendingJob;
    pendingJob.m_job = new Job(job);
    pendingJob.m_priority = priority;
    pendingJob.m_counter = counter;

    if (counter)
    {
        counter->Increment();
    }

    m_pendingTasksLock.Lock();
    m_pendingTasks.push(std::move(pendingJob));
    m_pendingTasksLock.Unlock();
}
