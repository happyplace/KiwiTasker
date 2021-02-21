#pragma once

#include <inttypes.h>

#include <bits/pthreadtypes.h>

namespace kiwi
{
struct Context;

class SchedulerImpl
{
public:
    SchedulerImpl();
    ~SchedulerImpl();

    SchedulerImpl(const SchedulerImpl&) = delete;
    SchedulerImpl& operator=(const SchedulerImpl&) = delete;

    int32_t GetCPUCount() const; 

    // returns the index of the current worker thread.
    // THIS SHOULD ONLY BE CALLED FROM WORKER THREADS
    int32_t GetWorkerThreadIndex() const;

    void CreateThread(const char* threadName, int32_t threadAffinity, void *(*threadFunction) (void *), void* threadFunctionArg);
    
    void ShutdownWorkerThreads();

    // should be called on worker threads after the thread is created
    void BlockSignalsOnThread();

    void SetContextInstructionAndStack(Context* context, void* instruction, void* stack) const;
    void SetContextParameters(Context* context, void* param0, void* param1, void* param2) const;

    char* GetStackPointerForStackBuffer(char* stackBuffer) const;

private:
    pthread_t* m_workerThreadIds;
    int m_workerThreadIdCount;
};
}