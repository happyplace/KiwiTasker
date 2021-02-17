#pragma once

#include <atomic>

namespace kiwi
{
class SpinLock
{
public:
    SpinLock() {}

    SpinLock(SpinLock const&) = delete;
    SpinLock& operator=(SpinLock const&) = delete;

    void Lock()
    {
        while (m_lock.test_and_set(std::memory_order_acquire))
            ;
    }

    void Unlock()
    {
        m_lock.clear(std::memory_order_release);
    }

private:
    std::atomic_flag m_lock;
};
}
