// #include <iostream>

// #include <atomic>

// #include <unistd.h>

// #include "kiwi/Job.h"
// #include "kiwi/Scheduler.h"

// void EndApplicationJob(kiwi::Scheduler* scheduler, void* arg)
// {
//     // convert the arg into a bool so we can change the value
//     std::atomic_bool* quit = reinterpret_cast<std::atomic_bool*>(arg); 

//     //printf("EndApplicationJob - Started\n");

//     //printf("EndApplicationJob - Completed\n");

//     // we're done running our job, change to true so the application will end
//     quit->store(true);
// }

// int main(int /*argc*/, char** /*argv*/)
// {
//     kiwi::Scheduler scheduler;
//     scheduler.Init();

//     std::atomic_bool quit;
//     quit.store(false);

//     kiwi::Job* job = new kiwi::Job();

//     // entry point function for the job, it's expected to take the Scheduler and one argument
//     job->m_function = EndApplicationJob;

//     // pass pointer to quit variable so we can change it true when our job completes
//     // if a pointer is used by a job that pointer needs to remain valid until the job is completed, quit exists
//     // on the stack so the memory will be valid.
//     job->m_arg = &quit;

//     // add job to the scheduler as a normal priority, if it's an important job we can run it as high priority
//     // or we can use low if it's minor task not required by the main loop
//     scheduler.AddJob(job, kiwi::JobPriority::Normal);

//     // once the job has been added to the scheduler it's save to delete, because a copy is kept
//     // remember that any pointers, that this job may have access to needs to be valid until the job
//     // completes execution
//     delete job;
//     job = nullptr;

//     // loop on quit until it's value is changed inside of EndApplicationJob
//     while (!quit.load())
//     {
//         sleep(1);
//     }

//     return 0;
// }


// assembled version of get_context_linux.s
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

// assembled version of set_context_linux.s
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

// assembled version of swap_context_unix.s
__attribute__((section(".text#")))
static unsigned char swap_context_code[] = {
    0x4c, 0x8b, 0x04, 0x24,
    0x4c, 0x89, 0x07,
    0x4c, 0x8d, 0x44, 0x24, 0x08,
    0x4c, 0x89, 0x47, 0x08,
    0x48, 0x89, 0x5f, 0x10,
    0x48, 0x89, 0x6f, 0x18,
    0x4c, 0x89, 0x67, 0x20,
    0x4c, 0x89, 0x6f, 0x28,
    0x4c, 0x89, 0x77, 0x30,
    0x4c, 0x89, 0x7f, 0x38,
    0x4c, 0x8b, 0x06,
    0x48, 0x8b, 0x66, 0x08,
    0x48, 0x8b, 0x5e, 0x10,
    0x48, 0x8b, 0x6e, 0x18,
    0x4c, 0x8b, 0x66, 0x20,
    0x4c, 0x8b, 0x6e, 0x28,
    0x4c, 0x8b, 0x76, 0x30,
    0x4c, 0x8b, 0x7e, 0x38,
    0x41, 0x50,
    0x48, 0x8b, 0x7e, 0x40,
    0x48, 0x8b, 0x56, 0x50,
    0x48, 0x8b, 0x76, 0x48,
    0x31, 0xc0,
    0xc3
};

struct Context {
    void *rip, *rsp;
    void *rbx, *rbp, *r12, *r13, *r14, *r15;
    void *rdi, *rsi, *rdx;
};

#include <assert.h>

#ifndef PTRD_ERR_HNDLR
#define PTRD_ERR_HNDLR(x) \
{ \
	int pth_result__ = (x); \
	if (pth_result__ != 0) \
	{ \
        assert(!#x); \
        return 1; \
	} \
}
#endif // PTRD_ERR_HNDLR(x)

#include <iostream>

#include <pthread.h>
#include <signal.h>
#include <bits/pthreadtypes.h>

static void (*get_context)(Context* context) = (void (*)(Context*))get_context_code;

static void (*set_context)(Context* context) = (void (*)(Context*))set_context_code;

static void (*swap_context)(Context* oldContext, Context* newContext) = (void (*)(Context*,Context*))swap_context_code;

#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

constexpr uint32_t stackSize = 512;

struct Fiber
{
    char* stackBuffer[stackSize];
};

struct Jobs
{
    using JobFunc = void (*)(void* arg);

    JobFunc m_function;
    void* m_arg;
    Fiber* m_fiber; 
};

struct WorkerData
{
    std::queue<Jobs>* m_queue;
    std::condition_variable* m_cv;
    std::mutex* m_m;
    std::atomic_bool* m_exit;
};

struct JobArg
{
    const char* message;
};

void FiberStart(Jobs* job, Context* returnContext)
{
    job->m_function(job->m_arg);

    // return to fiber worker main function
    set_context(returnContext);

    assert(!"We should never get here");
}

void GotHere(void* arg)
{
    JobArg* line = reinterpret_cast<JobArg*>(arg);

    printf("hello there\n");
    printf("%s\n", line->message);
}

// void FiberStart(Scheduler* scheduler, kiwi::Fiber* fiber)
// {
//     fiber->m_job.m_function(scheduler, fiber->m_job.m_arg);

//     // return fiber to pool
//     scheduler->GetFiberWorkerStorage()->m_fiberPool->ReturnFiber(fiber);

//     // return to fiber worker main function
//     set_context(&scheduler->GetFiberWorkerStorage()->m_context);
    
//     assert(!"should never get here");
// }

#if __has_include(<valgrind/valgrind.h>)
#include <valgrind/valgrind.h>
#define KIWI_HAS_VALGRIND
#endif

void* worker_thread_entry(void* arg)
{
    WorkerData* data = reinterpret_cast<WorkerData*>(arg);

    sigset_t mask;
    sigfillset(&mask);
    int pth_result = pthread_sigmask(SIG_BLOCK, &mask, nullptr);
    assert(!pth_result);

    while (!data->m_exit->load())
    {
        std::unique_lock<std::mutex> lck(*data->m_m);
        if (data->m_queue->empty())
        {
            data->m_cv->wait(lck, [&data] { return !data->m_queue->empty() || data->m_exit->load(); });
        }

        if (data->m_exit->load())
        {
            break;
        }

        Jobs job = data->m_queue->front();
        data->m_queue->pop();

        lck.unlock();

        volatile bool returningFromJob = false;
        
        Context main = {0};
        get_context(&main);

        if (!returningFromJob)
        {
            // when we jump back from FiberStart we won't come back in here
            returningFromJob = true;

            //char *sp = (char*)(stackBuffer + sizeof(stackBuffer));
            char *sp = (char*)(job.m_fiber->stackBuffer + stackSize);

            // Align stack pointer on 16-byte boundary.
            sp = (char*)((uintptr_t)sp & -16L);

            // Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
            // our stack pointer because 128 is a multiple of 16. The Red Zone must also be
            // 16-byte aligned.
            sp -= 128;

            Context funcJmp = {0};
            funcJmp.rip = (void*)FiberStart;
            funcJmp.rsp = (void*)sp;
            funcJmp.rdi = (void*)&job; 
            funcJmp.rsi = (void*)&main;

#if defined(KIWI_HAS_VALGRIND)
            // Before context switch, register our stack with Valgrind.
            unsigned stack_id = VALGRIND_STACK_REGISTER(job.m_fiber->stackBuffer, job.m_fiber->stackBuffer + stackSize);
#endif

            swap_context(&main, &funcJmp);

#if defined(KIWI_HAS_VALGRIND)
            // We've returned from the context switch, we can now throw that stack out.
            VALGRIND_STACK_DEREGISTER(stack_id);
#endif
        }
    }

    return NULL;
}

const char* message = "General Kenobi";

int main(int /*argc*/, char** /*argv*/)
{
    std::queue<Jobs> queue;
    std::condition_variable cv;
    std::mutex m;
    std::atomic_bool exit = ATOMIC_VAR_INIT(false);

    Fiber* fibers = new Fiber[1];

    JobArg arg;
    arg.message = message;

    Jobs job = {0};
    job.m_arg = &arg;
    job.m_function = GotHere;
    job.m_fiber = &fibers[0];
    queue.push(std::move(job));

    WorkerData workerData; //= new WorkerData();
    workerData.m_queue = &queue;
    workerData.m_cv = &cv;
    workerData.m_m = &m;
    workerData.m_exit = &exit;

    cpu_set_t cpuset;
    sched_getaffinity(0, sizeof(cpuset), &cpuset);
    int32_t cpuCount = static_cast<int32_t>(CPU_COUNT(&cpuset));

    pthread_t* workerThreads = new pthread_t[cpuCount];    

    for (int32_t i = 0; i < cpuCount; ++i)
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        cpu_set_t cpuAffinity;
        CPU_ZERO(&cpuAffinity);
        CPU_SET(static_cast<int>(i), &cpuAffinity);
        PTRD_ERR_HNDLR(pthread_attr_setaffinity_np(&attr, sizeof(cpuAffinity), &cpuAffinity));    

        PTRD_ERR_HNDLR(pthread_create(&workerThreads[i], &attr, worker_thread_entry, &workerData));

        PTRD_ERR_HNDLR(pthread_setname_np(workerThreads[i], "Worker Thread"));

        PTRD_ERR_HNDLR(pthread_attr_destroy(&attr));
    }
    //volatile int x = 0;

    //char *sp = (char*)(stackBuffer + sizeof(stackBuffer));
    //char *sp = (char*)(fibers[0].stackBuffer + 512);

    // Align stack pointer on 16-byte boundary.
    //sp = (char*)((uintptr_t)sp & -16L);

    // Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
    // our stack pointer because 128 is a multiple of 16. The Red Zone must also be
    // 16-byte aligned.
    //sp -= 128;

    //Context main = {0};
    //get_context(&main);

    //printf("hi\n");

    // if (x == 0)
    // {
    //     x = 1;

    //     Context funcJmp = {0};
    //     funcJmp.rip = (void*)GotHere;
    //     funcJmp.rsp = (void*)sp;
    //     funcJmp.rdi = (void*)&main; 
    //     set_context(&funcJmp);
    // }

    exit.store(true);

    std::unique_lock<std::mutex> lck(m);
    cv.notify_all();
    lck.unlock();

    for (int32_t i = 0; i < cpuCount; ++i)
    {
        PTRD_ERR_HNDLR(pthread_join(workerThreads[i], NULL));
    }

    delete[] workerThreads;

    delete[] fibers;

    return 0;
}
