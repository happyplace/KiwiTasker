#include "SchedulerImpl_unix.h"

#include <assert.h>

#include <pthread.h>

#ifndef PTRD_ERR_HNDLR
#define PTRD_ERR_HNDLR(x) \
{ \
	int pth_result__ = (x); \
	if (pth_result__ != 0) \
	{ \
        assert(!#x); \
        return; \
	} \
}
#endif // PTRD_ERR_HNDLR(x)

using namespace kiwi;

SchedulerImpl::SchedulerImpl()
{
    cpu_set_t cpuset;
    sched_getaffinity(0, sizeof(cpuset), &cpuset);
    m_workerThreadIdCount = static_cast<int32_t>(CPU_COUNT(&cpuset));

    m_workerThreadIds = new pthread_t[m_workerThreadIdCount];    
}

SchedulerImpl::~SchedulerImpl()
{
    for (int32_t i = 0; i < m_workerThreadIdCount; ++i)
    {
        PTRD_ERR_HNDLR(pthread_join(m_workerThreadIds[i], NULL));
    }
    delete[] m_workerThreadIds;
}

int32_t SchedulerImpl::GetCPUCount() const
{
    return m_workerThreadIdCount;
}

void SchedulerImpl::CreateThread(const char* threadName, int32_t threadAffinity, void *(*threadFunction) (void *), void* threadFunctionArg)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    cpu_set_t cpuAffinity;
    CPU_ZERO(&cpuAffinity);
    CPU_SET(static_cast<int>(threadAffinity), &cpuAffinity);
    PTRD_ERR_HNDLR(pthread_attr_setaffinity_np(&attr, sizeof(cpuAffinity), &cpuAffinity));    

    PTRD_ERR_HNDLR(pthread_create(&m_workerThreadIds[threadAffinity], &attr, threadFunction, threadFunctionArg));

    pthread_attr_destroy(&attr);

    PTRD_ERR_HNDLR(pthread_setname_np(m_workerThreadIds[threadAffinity], threadName));
}
