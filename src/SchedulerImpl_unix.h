#pragma once

#include <inttypes.h>

#include <bits/pthreadtypes.h>

namespace kiwi
{
class SchedulerImpl
{
public:
    SchedulerImpl();
    ~SchedulerImpl();

    SchedulerImpl(const SchedulerImpl&) = delete;
    SchedulerImpl& operator=(const SchedulerImpl&) = delete;

    int32_t GetCPUCount() const; 

    void CreateThread(const char* threadName, int32_t threadAffinity, void *(*threadFunction) (void *), void* threadFunctionArg);

private:
    pthread_t* m_workerThreadIds;
    int m_workerThreadIdCount;
};
}
