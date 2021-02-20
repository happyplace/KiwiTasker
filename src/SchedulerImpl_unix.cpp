#include "SchedulerImpl_unix.h"

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

// assembled version of get_context_linux.s
// this needs to be manually updated if the contents of get_context_linux.s changes
__attribute__((section(".text#")))
static unsigned char get_context_code[] = {
#if defined(__x86_64__)
    0x4c, 0x8b, 0x04, 0x24,
    0x4c, 0x89, 0x07,
    0x4c, 0x8d, 0x44, 0x24, 0x08,
    0x4c, 0x89, 0x47, 0x08,
    0x48, 0x89, 0x57, 0x10,
    0x48, 0x89, 0x6f, 0x18,
    0x4c, 0x89, 0x67, 0x20,
    0x4c, 0x89, 0x6f, 0x28,
    0x4c, 0x89, 0x77, 0x30,
    0x4c, 0x89, 0x7f, 0x38,
    0x31, 0xc0,
    0xc3
#else
#error get context does not exist for your cpu architecture
#endif
};

// assembed version of set_context_linux.s
// this needs to be manually updated if the contents of set_context_linux.s changes
__attribute__((section(".text#")))
static unsigned char set_context_code[] = {
#if defined(__x86_64__)
    0x4c, 0x8b, 0x07,
    0x48, 0x8b, 0x67, 0x08,
    0x48, 0x8b, 0x5f, 0x10,
    0x48, 0x8b, 0x6f, 0x18,
    0x4c, 0x8b, 0x67, 0x20,
    0x4c, 0x8b, 0x6f, 0x28,
    0x4c, 0x8b, 0x77, 0x30,
    0x4c, 0x8b, 0x7f, 0x38,
    0x41, 0x50,
    0x48, 0x8b, 0x77, 0x48,
    0x48, 0x8b, 0x57, 0x50,
    0x48, 0x8b, 0x7f, 0x40,
    0x31, 0xc0,
    0xc3
#else
#error set context does not exist for your cpu architecture
#endif
};

static void (*get_context)(kiwi::Context*) = (void (*)(kiwi::Context*))get_context_code;
static void (*set_context)(kiwi::Context*) = (void (*)(kiwi::Context*))set_context_code;

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
    m_workerThreadIds = nullptr;
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
    char *sp = (char*)(stackBuffer + sizeof(stackBuffer));

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

void SchedulerImpl::SetContext(Context* context) const
{
    set_context(context);
}

void SchedulerImpl::GetContext(Context* context) const
{
    get_context(context);
}
