#include "kiwi/SchedulerImpl.h"

#include <assert.h>

#ifdef KIWI_HAS_VALGRIND
#include <valgrind/valgrind.h>
#endif // KIWI_HAS_VALGRIND

#include "kiwi/ContextFunctions.h"
#include "kiwi/FiberWorkerStorage.h"
#include "kiwi/Scheduler.h"

using namespace kiwi;

SchedulerImpl::SchedulerImpl()
    : m_queueHigh(KiwiConfig::schedulerQueueSize)
    , m_queueNormal(KiwiConfig::schedulerQueueSize)
    , m_queueLow(KiwiConfig::schedulerQueueSize)
    , m_pendingFiber(KiwiConfig::schdulerPendingFiberArraySize)    
    , m_waitingFibers(KiwiConfig::schedulerFiberPoolSize)
    , m_fiberPool(KiwiConfig::schedulerFiberPoolSize)
{
}

SchedulerImpl::~SchedulerImpl()
{
    if (m_initialized)
    {
        m_workerExit.store(true);
        m_workerConditionVariable.notify_all();
        m_threadImpl.JoinWorkerThreads();

        kiwi::DestroyFiberWorkerStorage();
    }
}

FiberWorkerStorage* GetFiberWorkerStorage(SchedulerWorkerStartParams* params)
{
    return GetFiberWorkerStorage(params->m_schedulerImpl->GetThreadImpl().GetWorkerThreadIndex());
}

void FiberStart(SchedulerWorkerStartParams* params)
{
    kiwi::Job& job = GetFiberWorkerStorage(params)->m_fiber->m_job;
    job.m_function(params->m_scheduler, job.m_arg);
    
#if defined(KIWI_HAS_VALGRIND)
    // we're returning the fiber to the pool so we can deregister the stack_id
    VALGRIND_STACK_DEREGISTER(GetFiberWorkerStorage(params)->m_fiber->stack_id);
#endif // defined(KIWI_HAS_VALGRIND)

    params->m_schedulerImpl->ReturnFiber(GetFiberWorkerStorage(params)->m_fiber);

    //set_context(&GetFiberWorkerStorage(params)->m_context);
}

WORKER_THREAD_DEFINITION(arg)
{
    SchedulerWorkerStartParams* params = reinterpret_cast<SchedulerWorkerStartParams*>(arg);
    
    params->m_schedulerImpl->GetThreadImpl().BlockSignalsOnWorkerThread();

    //get_context(&GetFiberWorkerStorage(params)->m_context);

    while (!params->m_workerExit->load())
    {
        Fiber* fiber = nullptr;
        bool resumeFiber = false;
        if (params->m_schedulerImpl->GetNextAvailableFiber(&fiber, resumeFiber))
        {
            if (resumeFiber)
            {
                GetFiberWorkerStorage(params)->m_fiber = fiber;
                //set_context(&fiber->m_context);
            }
            else
            {
                char* fiberStackPointer = fiber->m_stack;
                char *sp = (char*)(fiberStackPointer + KiwiConfig::fiberStackSize);
                
                // Align stack pointer on 16-byte boundary.
                sp = (char*)((uintptr_t)sp & -16L);

                // Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
                // our stack pointer because 128 is a multiple of 16. The Red Zone must also be
                // 16-byte aligned.
                sp -= 128;

#if defined(KIWI_HAS_VALGRIND)
                // Before context switch, register our stack with Valgrind.
                fiber->stack_id = VALGRIND_STACK_REGISTER(fiberStackPointer, fiberStackPointer + KiwiConfig::fiberStackSize);
#endif // defined(KIWI_HAS_VALGRIND)

                kiwi::SetupContext(&fiber->m_context, (void*)FiberStart, sp, params);

                GetFiberWorkerStorage(params)->m_fiber = fiber;
                //set_context(&fiber->m_context);
            }

            continue;
        }

        std::unique_lock<std::mutex> cvLock(*params->m_workerMutex);
        params->m_workerConditionVariable->wait(cvLock);
    }

    WORKER_THREAD_RETURN_STATEMENT;
}

void SchedulerImpl::Init(Scheduler* scheduler)
{
    assert(!m_initialized && "Init() was called more than once");
    m_initialized = true;

    constexpr size_t threadNameBufferSize = 128;
    constexpr int threadNameMaxSize = 16;
    char threadNameBuffer[threadNameBufferSize];

    m_threadStartParams.m_scheduler = scheduler;
    m_threadStartParams.m_schedulerImpl = this;
    m_threadStartParams.m_workerConditionVariable = &m_workerConditionVariable;
    m_threadStartParams.m_workerExit = &m_workerExit;
    m_threadStartParams.m_workerMutex = &m_workerMutex;

    const int32_t cpuCount = m_threadImpl.GetCpuCount();

    kiwi::CreateFiberWorkerStorage(cpuCount);

    for (int32_t i = 0; i < cpuCount; ++i)
    {
        int result = snprintf(threadNameBuffer, threadNameBufferSize, "Kiwi Worker %i", i);
        // if the resulting thread name is going to be larger than the max thread name size, truncate the name
        if (result > threadNameMaxSize)
        {
            threadNameBuffer[threadNameMaxSize - 1] = '\0';
        }
        m_threadImpl.CreateThread(threadNameBuffer, i, SchedulerWorkerThread, &m_threadStartParams);
    }
}

void SchedulerImpl::AddJobs(const Job* jobs, const uint32_t size, const JobPriority priority /*= JobPriority::Normal*/, Counter* counter /*= nullptr*/)
{
    m_queueSpinLock.Lock();
    
    Queue<PendingJob>* queue = nullptr;
    switch (priority)
    {
        case JobPriority::High:
            queue = &m_queueHigh;
            break;
        case JobPriority::Normal:
            queue = &m_queueNormal;
            break;
        case JobPriority::Low:
            queue = &m_queueLow;
            break;
        default:
#ifdef KIWI_SCHEDULER_ERROR_CHECKING
            assert(!"Job priority does not exist, Normal priority will be used.");
#endif // KIWI_SCHEDULER_ERROR_CHECKING
            queue = &m_queueNormal;
            break;
    }

    PendingJob pendingJob;
    pendingJob.m_counter = counter;
    const bool hasCounter = counter != nullptr;
    for (uint32_t i = 0; i < size; ++i)
    {
        if (hasCounter)
        {
            counter->Increment();
        }
        pendingJob.m_job = jobs[i];
        queue->Push(pendingJob);
    }

    m_queueSpinLock.Unlock();

    if (size > 1)
    {
        m_workerConditionVariable.notify_all();
    }
    else
    {
        m_workerConditionVariable.notify_one();
    }
}

bool SchedulerImpl::GetNextAvailableFiber(Fiber** outFiber, bool& outResume)
{
    outResume = false;

    m_queueSpinLock.Lock();

    if (!m_queueHigh.IsEmpty())
    {
        PendingJob pendingJob;
        bool result = m_queueHigh.TryGetAndPopFront(&pendingJob);

#ifdef KIWI_SCHEDULER_ERROR_CHECKING
        assert(result && "Did someone change this list when we had the lock on it?");
#endif // KIWI_SCHEDULER_ERROR_CHECKING

        m_queueSpinLock.Unlock();
        
        (*outFiber) = m_fiberPool.GetFiber();
        (*outFiber)->m_job = pendingJob.m_job;
        (*outFiber)->m_counter = pendingJob.m_counter;

        return true;
    }

    if (!m_pendingFiber.IsEmpty())
    {
        (*outFiber) = m_pendingFiber[0];
        m_pendingFiber.Remove(0);

        m_queueSpinLock.Unlock();

        outResume = true;
        return true;
    }

    PendingJob pendingJob;
    bool gotJob = m_queueNormal.TryGetAndPopFront(&pendingJob);
    gotJob = !gotJob ? m_queueLow.TryGetAndPopFront(&pendingJob) : gotJob;
    
    m_queueSpinLock.Unlock();

    if (gotJob)
    {
        (*outFiber) = m_fiberPool.GetFiber();
        (*outFiber)->m_job = pendingJob.m_job;
        (*outFiber)->m_counter = pendingJob.m_counter;

        return true;
    }

    return false;
}

void SchedulerImpl::ReturnFiber(Fiber* fiber)
{
    if (fiber->m_counter)
    {
        int64_t value = fiber->m_counter->Decrement();
        
        // we're getting the value before we decrement so we need to subtract 1 to get the value
        value--;

        m_waitingFiberLock.Lock();
        for (int32_t i = 0; i < m_waitingFibers.GetSize(); ++i)
        {
            WaitingFiber& waitingFiber = m_waitingFibers[i];
            if (waitingFiber.m_counter == fiber->m_counter)
            {
                if (waitingFiber.m_targetValue == value)
                {
                    m_queueSpinLock.Lock();
                    m_pendingFiber.PushBack(waitingFiber.m_fiber);
                    m_queueSpinLock.Unlock();

                    m_waitingFibers.Remove(i);
                    --i;
                }
            }
        }
        m_waitingFiberLock.Unlock();
    }

    m_fiberPool.ReturnFiber(fiber);
}

#ifdef KIWI_SCHEDULER_ERROR_CHECKING
void SchedulerImpl::WakeAnyFibersWaitingOnCounter(Counter* counter)
{
    m_waitingFiberLock.Lock();
    for (int32_t i = 0; i < m_waitingFibers.GetSize(); ++i)
    {
        WaitingFiber& waitingFiber = m_waitingFibers[i];
        if (waitingFiber.m_fiber->m_counter == counter)
        {
            assert(!"a counter was deleted while there was still fibers waiting on the counter");

            m_queueSpinLock.Lock();
            m_pendingFiber.PushBack(waitingFiber.m_fiber);
            m_queueSpinLock.Unlock();

            m_waitingFibers.Remove(i);
            --i;
        }
    }
    m_waitingFiberLock.Unlock();
}
#endif // KIWI_SCHEDULER_ERROR_CHECKING

void SchedulerImpl::WaitForCounter(Counter* counter, int64_t value /*= 0*/)
{
#ifdef KIWI_SCHEDULER_ERROR_CHECKING
    assert(counter);
#endif // #ifdef KIWI_SCHEDULER_ERROR_CHECKING

    m_waitingFiberLock.Lock();
    if (counter->GetValue() != value)
    {
        WaitingFiber waitingFiber;
        waitingFiber.m_fiber = GetFiberWorkerStorage(m_threadImpl.GetWorkerThreadIndex())->m_fiber;
        waitingFiber.m_targetValue = value;
        waitingFiber.m_counter = counter;
        m_waitingFibers.PushBack(waitingFiber);
        m_waitingFiberLock.Unlock();
        //swap_context(&waitingFiber.m_fiber->m_context, &GetFiberWorkerStorage(m_threadImpl.GetWorkerThreadIndex())->m_context);
    }
    else
    {
        // if the counter is already at the value unlock and return
        m_waitingFiberLock.Unlock();
    }
}
