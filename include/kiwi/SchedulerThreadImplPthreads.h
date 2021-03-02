#pragma once

#include <pthread.h>
#include <inttypes.h>

#define WORKER_THREAD_DEFINITION(X) void* SchedulerWorkerThread(void* X)
#define WORKER_THREAD_RETURN_STATEMENT return NULL

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

    // this should be called by each worker thread to prevent receiving any signals
    void BlockSignalsOnWorkerThread() const;

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
