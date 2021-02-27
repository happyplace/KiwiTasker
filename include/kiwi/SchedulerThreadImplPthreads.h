#pragma once

#include <pthread.h>
#include <inttypes.h>

namespace kiwi
{
class SchedulerThreadImpl
{
public:
    SchedulerThreadImpl();
    ~SchedulerThreadImpl();

    SchedulerThreadImpl(const SchedulerThreadImpl&) = delete;
    SchedulerThreadImpl& operator=(const SchedulerThreadImpl&) = delete;

    void CreateThread(const char* threadName, int32_t threadAffinity, void *(*threadFunction) (void *), void* threadFunctionArg);

    void JoinWorkerThreads();

    // this uses the thread affinity to determine what the thread index is
    // this will only worker for scheduler worker threads.
    // Returns -1 if there is an error getting the thread index
    int32_t GetWorkerThreadIndex() const;

    // returns the physical cpu count reported by the threading library
    int32_t GetCpuCount() const;

private:
    pthread_t* m_workerThreads = nullptr;
};
}
