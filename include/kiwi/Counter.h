#pragma once

#include <atomic>
#include <inttypes.h>

namespace kiwi
{
class Scheduler;

class Counter
{
public:
    explicit Counter(Scheduler* schduler);
    ~Counter();

    Counter(const Counter&) = delete;
    Counter& operator=(const Counter&) = delete;

    // increments counter and returns the value it had BEFORE it was incremented
    int64_t Increment();

    // decrements counter and returns the value it had BEFORE it was decremented
    int64_t Decrement();

    int64_t GetValue() const { return m_value.load(); }

private:
    Scheduler* m_scheduler = nullptr;
    std::atomic<int64_t> m_value;
};
}
