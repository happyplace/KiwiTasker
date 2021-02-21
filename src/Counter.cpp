#include "kiwi/Counter.h"

#include "kiwi/Scheduler.h"

using namespace kiwi;

Counter::Counter(Scheduler* schduler)
    : m_scheduler(schduler)
{
}

Counter::~Counter()
{
    // check if there are waiting fibers and trigger CRITICAL ERROR in debug in release force wake up all waiting fibers
}

void Counter::CheckWaiting()
{
    // for (WaitingFiber& waitingFiber : m_waitingFibers)
    // {
    //     if (waitingFiber.m_value == m_value)
    //     {
    //         for (std::size_t i = 0; i < m_scheduler->m_waitList.size(); ++i)
    //         {
    //             if (m_scheduler->m_waitList[i] == waitingFiber.m_fiber)
    //             {
    //                 m_scheduler->m_waitList.erase(m_scheduler->m_waitList.begin() + i);
    //                 m_scheduler->m_readyList.push_back(waitingFiber.m_fiber);                        
    //                 --i;
    //             }
    //         }
    //         return;
    //         // remove from waiting list, put back in list                
    //     }
    // }
}
