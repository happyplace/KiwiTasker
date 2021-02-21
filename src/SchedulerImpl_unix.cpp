#include "kiwi/SchedulerImpl_unix.h"

#include <assert.h>

#include <pthread.h>
#include <signal.h>

#include "kiwi/Context.h"

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
    m_workerThreadIdCount = 1;//static_cast<int32_t>(CPU_COUNT(&cpuset));

    m_workerThreadIds = new pthread_t[m_workerThreadIdCount];    
}

SchedulerImpl::~SchedulerImpl()
{
    delete[] m_workerThreadIds;
}

void SchedulerImpl::ShutdownWorkerThreads()
{
    for (int32_t i = 0; i < m_workerThreadIdCount; ++i)
    {
        PTRD_ERR_HNDLR(pthread_join(m_workerThreadIds[i], NULL));
    }
}

int32_t SchedulerImpl::GetCPUCount() const
{
    return m_workerThreadIdCount;
}

int32_t SchedulerImpl::GetWorkerThreadIndex() const
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
            // assuming this is called from a worker thread only one cpu should set so we can return on the first true
            return i;
        }
    }

    return -1;
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

    PTRD_ERR_HNDLR(pthread_create(&m_workerThreadIds[threadAffinity], &attr, threadFunction, threadFunctionArg));

    PTRD_ERR_HNDLR(pthread_setname_np(m_workerThreadIds[threadAffinity], threadName));
}

void SchedulerImpl::BlockSignalsOnThread()
{
    // Don't permit signal delivery to this thread.
    sigset_t mask;
    sigfillset(&mask);
    PTRD_ERR_HNDLR(pthread_sigmask(SIG_BLOCK, &mask, nullptr));
}

void SchedulerImpl::SetContextInstructionAndStack(Context* context, void* instruction, void* stack) const
{
    context->rip = instruction;
    context->rsp = stack;
}

void SchedulerImpl::SetContextParameters(Context* context, void* param0, void* param1, void* param2) const
{
#if defined(__x86_64__)
    context->rdi = param0;
    context->rsi = param1;
    context->rdx = param2;
#else
#error SetContextParameters does not exist for your cpu architecture
#endif
}

char* SchedulerImpl::GetStackPointerForStackBuffer(char* stackBuffer) const
{
#if defined(__x86_64__)
    const size_t bufferSize = sizeof(stackBuffer);
    char *sp = (char*)(stackBuffer + bufferSize);

    // Align stack pointer on 16-byte boundary.
    sp = (char*)((uintptr_t)sp & -16L);

    // Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
    // our stack pointer because 128 is a multiple of 16. The Red Zone must also be
    // 16-byte aligned.
    sp -= 128;

    return sp;
#else
#error GetStackPointerForStack does not exist for your cpu architecture
return NULL;
#endif
}
