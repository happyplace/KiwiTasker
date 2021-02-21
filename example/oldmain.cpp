#include <iostream>
#include <assert.h>
#include <queue>

#include <pthread.h>

#include "kiwi/Scheduler.h"
#include "kiwi/Job.h"
#include "kiwi/Context.h"
#include "kiwi/FiberWorkerData.h"
#include "kiwi/Counter.h"

// assembled version of get_context_linux.s
// this needs to be manually updated if the contents of get_context_linux.s changes
__attribute__((section(".text#")))
static unsigned char get_context_code[] = {
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
};

static void (*get_context)(kiwi::Context*) = (void (*)(kiwi::Context*))get_context_code;

// assembed version of set_context_linux.s
// this needs to be manually updated if the contents of set_context_linux.s changes
__attribute__((section(".text#")))
static unsigned char set_context_code[] = {
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
};

static void (*set_context)(kiwi::Context*) = (void (*)(kiwi::Context*))set_context_code;

void DatFunctionBoy(kiwi::Scheduler* scheduler, void* data)
{
    int datData = *(reinterpret_cast<int*>(data));
    printf("%i)\n", datData);  
}

int GetFiberIndex()
{
    pthread_t thread = pthread_self();

    cpu_set_t cpuset;
    int result = pthread_getaffinity_np(thread, sizeof(cpuset), &cpuset);
    if (result != 0)
    {
        assert(false);
        return -1;
    }

    int cpuIndex = -1;
    for (int i = 0; i < CPU_SETSIZE; i++)
    {
        if (CPU_ISSET(i, &cpuset))
        {
            cpuIndex = i;
        }
    }

    assert(cpuIndex >= 0); // either the workers are setup incorrectly, or this function is being called from a non-fiber thread

    return cpuIndex;
}

void WaitForCounter(kiwi::Scheduler* scheduler, kiwi::Counter* counter, uint64_t value)
{
    kiwi::FiberWorkerData* workerData = &scheduler->m_fiberWorkerData[GetFiberIndex()];
    kiwi::Fiber* fiber = workerData->m_currentFiber;

    volatile bool returnedFromWait = false;

    get_context(&fiber->m_context);

    if (returnedFromWait)
    {
        returnedFromWait = true;

        // TODO: remove fiber from watch list

        set_context(&fiber->m_context);
    }
    else
    {
        counter->m_lock.Lock();
        if (counter->m_value != value)
        {
            scheduler->m_waitList.push_back(fiber);

            kiwi::Counter::WaitingFiber waitingFiber;
            waitingFiber.m_value = value;
            waitingFiber.m_fiber = fiber;
            counter->m_waitingFibers.push_back(std::move(waitingFiber));

            counter->m_lock.Unlock();

            kiwi::FiberWorkerData* workerData = &scheduler->m_fiberWorkerData[GetFiberIndex()];
            set_context(&workerData->m_fiberWorkerReturn);
        }  
    }
}

void TheOtherJob(kiwi::Scheduler* scheduler, void* data)
{
    pthread_t thread = pthread_self();

    cpu_set_t cpuset;
    int result = pthread_getaffinity_np(thread, sizeof(cpuset), &cpuset);
    if (result != 0)
    {
        assert(false);
        return;
    }

    int cpuIndex = -1;
    for (int i = 0; i < CPU_SETSIZE; i++)
    {
        if (CPU_ISSET(i, &cpuset))
        {
            cpuIndex = i;
        }
    }

    printf("TheOtherJob: I'm the other job that needs to finish first. I'm on CPU: %i\n", cpuIndex);
}

void SpawnJobJob(kiwi::Scheduler* scheduler, void* data)
{
    printf("SpawnJobJob: I'm spawning a job.\n");

    kiwi::Counter counter(scheduler);

    kiwi::Job otherJob;
    otherJob.m_function = TheOtherJob;
    otherJob.m_arg = nullptr;
    scheduler->AddJob(otherJob, kiwi::JobPriority::Normal, &counter);

    WaitForCounter(scheduler, &counter, 0);

    printf("SpawnJobJob: The other job should have finished.\n");
}

void fiber_start(kiwi::Scheduler* scheduler, kiwi::FiberWorkerData* data, kiwi::Fiber* fiber)
{
    kiwi::Job* job = fiber->m_job;
    job->m_function(scheduler, job->m_arg);

    // the job has completed running and there is no more access to this job, it can be safely
    // deleted now
    delete job;
    fiber->m_job = nullptr;

    // if we have a counter attached to ourselve decrement since this fiber is done
    if (fiber->m_counter)
    {
        fiber->m_counter->Decrement();
    }

    data->m_fiberPoolStatus.set(data->m_fiberPoolIndex, false);
    data->m_currentFiber = nullptr;

    // return to fiber worker after executing thread
    set_context(&data->m_fiberWorkerReturn);
}

static void* start_fiberWorker(void* arg)
{
    kiwi::FiberWorkerData* data = reinterpret_cast<kiwi::FiberWorkerData*>(arg);

    get_context(&data->m_fiberWorkerReturn);  

    while (!data->m_scheduler->m_pendingTasks.empty())
    {        
        kiwi::Fiber* fiber = nullptr;
        for (std::size_t i = 0; i < 2; ++i)
        {
            if (!data->m_fiberPoolStatus.test(i))
            {
                data->m_fiberPoolStatus.set(i);
                data->m_fiberPoolIndex = i;
                fiber = &data->m_fiberPool[i];
                break;
            }
        }
        assert(fiber != nullptr); // what???
        data->m_currentFiber = fiber;

        {
            data->m_scheduler->m_pendingTasksLock.Lock();
            kiwi::Scheduler::PendingJob pendingJob = data->m_scheduler->m_pendingTasks.front();
            data->m_scheduler->m_pendingTasks.pop();
            data->m_scheduler->m_pendingTasksLock.Unlock();

            fiber->m_job = pendingJob.m_job;
            fiber->m_counter = pendingJob.m_counter;
        }

        char *sp = (char*)(fiber->m_stack + sizeof(fiber->m_stack));

        // Align stack pointer on 16-byte boundary.
        sp = (char*)((uintptr_t)sp & -16L);

        // Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
        // our stack pointer because 128 is a multiple of 16. The Red Zone must also be
        // 16-byte aligned.
        sp -= 128;

        fiber->m_context = {0};
        fiber->m_context.rip = (void*)fiber_start;
        fiber->m_context.rsp = (void*)sp;
        fiber->m_context.rdi = data->m_scheduler;
        fiber->m_context.rsi = data;
        fiber->m_context.rdx = fiber;
        set_context(&fiber->m_context);
    }

    while (!data->m_scheduler->m_readyList.empty())
    { 
        std::list<kiwi::Fiber*>& readyList = data->m_scheduler->m_readyList;

        kiwi::Fiber* fiber = readyList.front();
        readyList.pop_front();

        set_context(&fiber->m_context);
    }

    return NULL;
}

#include <unistd.h>

int main(int argc, char** argv)
{
    kiwi::Scheduler scheduler;
    scheduler.Init();

    kiwi::Job jobs[2];

    jobs[0].m_function = DatFunctionBoy;
    int job1Arg = 1337;
    jobs[0].m_arg = &job1Arg;

    jobs[1].m_function = SpawnJobJob;

    scheduler.AddJob(jobs[0]);
    scheduler.AddJob(jobs[1]);

    scheduler.m_fiberWorkerData = new kiwi::FiberWorkerData[1];
    scheduler.m_fiberWorkerData[0].m_scheduler = &scheduler;

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    cpu_set_t cpuAffinity;
    CPU_ZERO(&cpuAffinity);
    CPU_SET(0, &cpuAffinity);
    int result = pthread_attr_setaffinity_np(&attr, sizeof(cpuAffinity), &cpuAffinity);
    if (result != 0)
    {
        assert(false);
        return 1;
    }

    pthread_t worker;
    result = pthread_create(&worker, &attr, start_fiberWorker, &scheduler.m_fiberWorkerData[0]);
    if (result != 0)
    {
        assert(false);
        return 1;
    }

    result = pthread_setname_np(worker, "Fiber Worker 0");
    if (result != 0)
    {
        assert(false);
        return 1;
    }

    sleep(5);    

    return 0;
}
