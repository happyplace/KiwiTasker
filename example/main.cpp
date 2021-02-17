#include <iostream>
#include <assert.h>
#include <queue>

#include <pthread.h>

#include "kiwi/Scheduler.h"

namespace kiwi
{
struct ContextUnix
{
    void *rip, *rsp;
    void *rbx, *rbp, *r12, *r13, *r14, *r15;
    void *rdi, *rsi, *rdx;
};
}

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

static void (*get_context)(kiwi::ContextUnix*) = (void (*)(kiwi::ContextUnix*))get_context_code;

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

static void (*set_context)(kiwi::ContextUnix*) = (void (*)(kiwi::ContextUnix*))set_context_code;

struct Job
{
    using JobFunc = void (*)(kiwi::Scheduler* scheduler, void* arg);

    JobFunc m_function;
    void* m_arg = nullptr; 
};

std::queue<Job*> pendingTasks;

void DatFunctionBoy(kiwi::Scheduler* scheduler, void* data)
{
    int datData = *(reinterpret_cast<int*>(data));
    printf("%i)\n", datData);  
}

void TheOtherJob(kiwi::Scheduler* scheduler, void* data)
{
    printf("I'm the other job that needs to finish first\n");
}

void SpawnJobJob(kiwi::Scheduler* scheduler, void* data)
{
    printf("I'm spawning a job.\n");
    printf("The other job should have finished.\n");
}

struct Fiber
{
    char m_stack[4096];
    kiwi::ContextUnix m_context;
    Job* m_job;
};

void fiber_start(kiwi::Scheduler* scheduler, kiwi::ContextUnix* returnContext, Fiber* fiber)
{
    Job* job = fiber->m_job;
    job->m_function(scheduler, job->m_arg);

    // the job has completed running and there is no more access to this job, it can be safely
    // deleted now
    delete job;
    fiber->m_job = nullptr;

    // TODO: we need to return the fiber to the pool

    // return to fiber worker after executing thread
    set_context(returnContext);
}

static void* start_fiberWorker(void* arg)
{
    kiwi::Scheduler scheduler;

    kiwi::ContextUnix fiberWorkerReturn;
    get_context(&fiberWorkerReturn);
    fiberWorkerReturn.rdi = arg;

    Fiber fiber;

    while (!pendingTasks.empty())
    {        
        fiber.m_job = pendingTasks.front();
        pendingTasks.pop();
    
        char *sp = (char*)(fiber.m_stack + sizeof(fiber.m_stack));

        // Align stack pointer on 16-byte boundary.
        sp = (char*)((uintptr_t)sp & -16L);

        // Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
        // our stack pointer because 128 is a multiple of 16. The Red Zone must also be
        // 16-byte aligned.
        sp -= 128;

        fiber.m_context = {0};
        fiber.m_context.rip = (void*)fiber_start;
        fiber.m_context.rsp = (void*)sp;
        fiber.m_context.rdi = &scheduler;
        fiber.m_context.rsi = &fiberWorkerReturn;
        fiber.m_context.rdx = &fiber;
        set_context(&fiber.m_context);
    }
    return NULL;
}

#include <unistd.h>

int main(int argc, char** argv)
{
    kiwi::Scheduler scheduler;

    Job* job1 = new Job();
    job1->m_function = DatFunctionBoy;
    int job1Arg = 1337;
    job1->m_arg = &job1Arg;

    Job* job2 = new Job();
    job2->m_function = SpawnJobJob;

    pendingTasks.push(job1);
    pendingTasks.push(job2);
    //pendingTasks.push(23);

    cpu_set_t cpuset;
    sched_getaffinity(0, sizeof(cpuset), &cpuset);
    int cpuCount = CPU_COUNT(&cpuset);

    pthread_t worker;
    int result = pthread_create(&worker, NULL, &start_fiberWorker, NULL);
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

    cpu_set_t cpuAffinity;
    CPU_ZERO(&cpuAffinity);
    CPU_SET(0, &cpuAffinity);
    result = pthread_setaffinity_np(worker, sizeof(cpuAffinity), &cpuAffinity);
    if (result != 0)
    {
        assert(false);
        return 1;
    }

    sleep(5);

    return 0;
}
