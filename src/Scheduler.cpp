#include "kiwi/Scheduler.h"

#include "kiwi/SchedulerImpl.h"

using namespace kiwi;

Scheduler::Scheduler()
    : m_impl(nullptr)
{    
}

Scheduler::~Scheduler()
{
    if (m_impl != nullptr)
    {
        delete m_impl;
    }
}

void Scheduler::Init()
{
    m_impl = new SchedulerImpl();
}

void Scheduler::AddJob(const Job* job, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    m_impl->AddJob(job, 1, priority, counter);
}

void Scheduler::AddJob(const Job* jobs, const uint32_t size, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    m_impl->AddJob(jobs, size, priority, counter);
}

void Scheduler::WaitForCounter(Counter* counter, uint64_t value /*= 0*/)
{
    m_impl->WaitForCounter(counter, value);
}
