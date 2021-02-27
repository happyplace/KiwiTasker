#include "kiwi/Counter.h"

#include "kiwi/Config.h"
#include "kiwi/Scheduler.h"
#include "kiwi/SchedulerImpl.h"

using namespace kiwi;

Counter::Counter(Scheduler* schduler)
    : m_scheduler(schduler)
{
    m_value.store(0);
}

Counter::~Counter()
{
#ifdef KIWI_SCHEDULER_ERROR_CHECKING
    m_scheduler->GetImpl()->WakeAnyFibersWaitingOnCounter(this);
#endif // KIWI_SCHEDULER_ERROR_CHECKING
}

int64_t Counter::Increment()
{
    return m_value.fetch_add(1);
}

int64_t Counter::Decrement()
{
    return m_value.fetch_sub(1);
}
