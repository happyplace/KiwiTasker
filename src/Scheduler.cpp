#include "kiwi/Scheduler.h"

#include <assert.h>

#include "kiwi/Config.h"
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
#ifdef KIWI_SCHEDULER_ERROR_CHECKING
    assert(m_impl == nullptr);
#endif // KIWI_SCHEDULER_ERROR_CHECKING

    if (m_impl == nullptr)
    {
        m_impl = new SchedulerImpl();
        m_impl->Init(this);
    }
}

void Scheduler::AddJobs(const Job* job, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    m_impl->AddJobs(job, 1, priority, counter);
}

void Scheduler::AddJobs(const Job* jobs, const uint32_t size, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    m_impl->AddJobs(jobs, size, priority, counter);
}

void Scheduler::WaitForCounter(Counter* counter, int64_t value /*= 0*/)
{
    m_impl->WaitForCounter(counter, value);
}
