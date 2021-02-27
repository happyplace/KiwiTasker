#include "kiwi/SchedulerThreadImplPthreads.h"

#include <assert.h>

#include <signal.h>

#ifndef PTRD_ERR_HNDLR
#define PTRD_ERR_HNDLR(x) \
{ \
	int pth_result__ = (x); \
	if (pth_result__ != 0) \
	{ \
        assert(!#x); \
	} \
}
#endif // PTRD_ERR_HNDLR(x)

using namespace kiwi;

SchedulerThreadImpl::SchedulerThreadImpl()
{
    m_workerThreads = new pthread_t[GetCpuCount()];
}

SchedulerThreadImpl::~SchedulerThreadImpl()
{
    delete[] m_workerThreads;
}

void SchedulerThreadImpl::JoinWorkerThreads()
{
    const int32_t cpuCount = GetCpuCount();
    for (int32_t i = 0; i < cpuCount; ++i)
    {
        PTRD_ERR_HNDLR(pthread_join(m_workerThreads[i], NULL));
    }
}

void SchedulerThreadImpl::CreateThread(const char* threadName, int32_t threadAffinity, void *(*threadFunction) (void *), void* threadFunctionArg)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    cpu_set_t cpuAffinity;
    CPU_ZERO(&cpuAffinity);
    CPU_SET(static_cast<int>(threadAffinity), &cpuAffinity);
    PTRD_ERR_HNDLR(pthread_attr_setaffinity_np(&attr, sizeof(cpuAffinity), &cpuAffinity));    

    PTRD_ERR_HNDLR(pthread_create(&m_workerThreads[threadAffinity], &attr, threadFunction, threadFunctionArg));

    PTRD_ERR_HNDLR(pthread_setname_np(m_workerThreads[threadAffinity], threadName));

    PTRD_ERR_HNDLR(pthread_attr_destroy(&attr));
}

int32_t SchedulerThreadImpl::GetWorkerThreadIndex() const
{
    pthread_t thread = pthread_self();

    cpu_set_t cpuset;
    int result = pthread_getaffinity_np(thread, sizeof(cpuset), &cpuset);
    if (result != 0)
    {
        assert(false);
        return -1;
    }

    for (int i = 0; i < CPU_SETSIZE; i++)
    {
        if (CPU_ISSET(i, &cpuset))
        {
            // assuming this is called from a worker thread, only one cpu should be set so we can return on the first true
            return i;
        }
    }

    assert(!"Why is no cpu set, is this being called outside of a scheduler thread?");
    return -1;
}

int32_t SchedulerThreadImpl::GetCpuCount() const
{
    cpu_set_t cpuset;
    sched_getaffinity(0, sizeof(cpuset), &cpuset);
    return static_cast<int32_t>(CPU_COUNT(&cpuset));
}

void SchedulerThreadImpl::BlockSignalsOnWorkerThread() const
{
    sigset_t mask;
    sigfillset(&mask);
    PTRD_ERR_HNDLR(pthread_sigmask(SIG_BLOCK, &mask, nullptr));
}
