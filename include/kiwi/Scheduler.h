#pragma once

namespace kiwi
{
class Scheduler
{
public:
    Scheduler();
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
};
}
