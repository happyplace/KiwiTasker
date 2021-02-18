#pragma once

#include <list>
#include <atomic>

#include "kiwi/SpinLock.h"

namespace kiwi
{
struct Fiber;
struct Scheduler;

class Counter
{
public:
    Counter(Scheduler* schduler);
    ~Counter();

    void Increment()
    {
        m_lock.Lock();
        m_value++;
        CheckWaiting();
        m_lock.Unlock();
    }

    void Decrement()
    {
        m_lock.Lock();
        m_value--;
        CheckWaiting();
        m_lock.Unlock();
    }

    uint64_t Value() const { return m_value.load(); }

//private:
    void CheckWaiting();

    Scheduler* m_scheduler = nullptr;

    kiwi::SpinLock m_lock;
    std::atomic<uint64_t> m_value;

    struct WaitingFiber
    {
        kiwi::Fiber* m_fiber;
        uint64_t m_value;
    };

    std::list<WaitingFiber> m_waitingFibers;
};
}
