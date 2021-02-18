#pragma once

namespace kiwi
{
class Scheduler;

struct Job
{
    using JobFunc = void (*)(kiwi::Scheduler* scheduler, void* arg);

    JobFunc m_function;
    void* m_arg = nullptr; 
};
}
