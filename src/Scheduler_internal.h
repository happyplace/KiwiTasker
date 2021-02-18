#pragma once

namespace kiwi
{
class Scheduler_internal
{
public:
    Scheduler_internal();
    ~Scheduler_internal();

    Scheduler_internal(const Scheduler_internal&) = delete;
    Scheduler_internal& operator=(const Scheduler_internal&) = delete;
};
}
