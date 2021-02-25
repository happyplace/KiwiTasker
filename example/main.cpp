// #include <iostream>

// #include <atomic>

#include <unistd.h>

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
#include <atomic>

#include "kiwi/FiberPool.h"
#include "kiwi/Fiber.h"
#include "kiwi/Queue.h"
#include "kiwi/SpinLock.h"

struct Jobs
{
    using JobFunc = void (*)(void* arg);

    JobFunc m_function;
    void* m_arg;
    kiwi::Fiber* m_fiber; 
};

struct WorkerData
{
    kiwi::SpinLock* m_queueLock;
    kiwi::Queue<Jobs>* m_queue;
    std::condition_variable* m_cv;
    std::mutex* m_m;
    std::atomic_bool* m_exit;
    kiwi::FiberPool* m_fiberPool;
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

static kiwi::SpinLock lock;

int32_t GetWorkerThreadIndex()
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

    return -1;
}

void GotHere(void* arg)
{
    JobArg* line = reinterpret_cast<JobArg*>(arg);

    lock.Lock();
    printf("hello there %i\n", GetWorkerThreadIndex());
    printf("%s\n", line->message);
    lock.Unlock();
}

void TheOtherHere(void* arg)
{
    lock.Lock();
    printf("The Other here %i\n", GetWorkerThreadIndex());
    lock.Unlock();
}

struct CounterTaskData
{
    kiwi::SpinLock* m_queueLock;
    kiwi::Queue<Jobs>* m_queue;
    kiwi::FiberPool* fiberPool;
    std::condition_variable* m_cv;
};

struct NumberData
{
    int32_t num;
};

void PrintNumberTask(void* arg)
{
    NumberData* num = reinterpret_cast<NumberData*>(arg);

    sleep(1); // super bad, but we want this task to take awhile so we can test counters

    lock.Lock();
    printf("PrintNumberTask: %i CPU: %i\n", num->num, GetWorkerThreadIndex());
    lock.Unlock();
}

void CounterTask(void* arg)
{
    CounterTaskData* data = reinterpret_cast<CounterTaskData*>(arg);

    lock.Lock();
    printf("Entered Counter Task %i\n", GetWorkerThreadIndex());
    lock.Unlock();

    NumberData* nums = new NumberData[25];
    Jobs* jobs = new Jobs[25];

    for (int i = 0; i < 25; ++i)
    {
        nums[i].num = i;
        jobs[i] = { PrintNumberTask, &nums[i], data->fiberPool->GetFiber() };
    }

    data->m_queueLock->Lock();
    for (int i = 0; i < 25; ++i)
    {
        data->m_queue->Push(jobs[i]);
    }
    data->m_queueLock->Unlock();

    data->m_cv->notify_all();

    delete[] jobs;

    lock.Lock();
    printf("Resumed because Counter finished %i\n", GetWorkerThreadIndex());
    lock.Unlock();

    //delete[] nums;
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
        //ck_barrier_mcs_subscribe(&barrier, &state);

        //ck_pr_inc_int(&barrier_wait);
        // if the queue is empty stall until it's ready
        //while (ck_pr_load_int(&barrier_wait) != 1)
        //{
        //    ck_pr_stall();
        //}

        //if (data->m_exit->load())
        //{
        //    break;
        //}

        //Jobs job = data->m_queue->front();
        //data->m_queue->pop();

        //lck.unlock();

        bool hasNode = false;
        //JobsQueueNode* node = nullptr;

        // ck_spinlock_fas_lock(data->m_queueLock);   
        // hasNode = !CK_STAILQ_EMPTY(data->m_queue);
        // if (hasNode)
        // {
        //     node = CK_STAILQ_FIRST(data->m_queue);
        //     CK_STAILQ_REMOVE_HEAD(data->m_queue, list_entry);
        // }
        // ck_spinlock_fas_unlock(data->m_queueLock);

        data->m_queueLock->Lock();
        Jobs node;
        hasNode = data->m_queue->TryGetAndPopFront(&node);
        data->m_queueLock->Unlock();

        if (!hasNode)
        {
            std::unique_lock<std::mutex> lck(*data->m_m);
            data->m_cv->wait(lck, [&data]
            {
                data->m_queueLock->Lock();
                const bool queueEmpty = data->m_queue->IsEmpty();
                data->m_queueLock->Unlock();

                return data->m_exit->load() || !queueEmpty;
            });
            
            // return to the start of the loop so we can check if we should shutdown the worker, check for new jobs, or go back
            // to sleep because some other worker got the task before we could
            continue;
        }

        volatile bool returningFromJob = false;
        
        Context main = {0};
        get_context(&main);

        if (!returningFromJob)
        {
            // when we jump back from FiberStart we won't come back in here
            returningFromJob = true;

            //char *sp = (char*)(stackBuffer + sizeof(stackBuffer));
            //char* address = node.m_fiber->m_stack;
            //(void)address;

            char *sp = (char*)(node.m_fiber->m_stack + KiwiConfig::fiberSmallStackSize);

            // Align stack pointer on 16-byte boundary.
            sp = (char*)((uintptr_t)sp & -16L);

            // Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
            // our stack pointer because 128 is a multiple of 16. The Red Zone must also be
            // 16-byte aligned.
            sp -= 128;

            Context funcJmp = {0};
            funcJmp.rip = (void*)FiberStart;
            funcJmp.rsp = (void*)sp;
            funcJmp.rdi = (void*)&node; 
            funcJmp.rsi = (void*)&main;

#if defined(KIWI_HAS_VALGRIND)
            // Before context switch, register our stack with Valgrind.
            unsigned stack_id = VALGRIND_STACK_REGISTER(node.m_fiber->m_stack, node.m_fiber->m_stack + KiwiConfig::fiberSmallStackSize);
#endif

            swap_context(&main, &funcJmp);

#if defined(KIWI_HAS_VALGRIND)
            // We've returned from the context switch, we can now throw that stack out.
            VALGRIND_STACK_DEREGISTER(stack_id);
#endif

            // return node to the pool
            // return fiber to the pool
        }
    }

    return NULL;
}

const char* message = "General Kenobi";

int main(int /*argc*/, char** /*argv*/)
{
    std::condition_variable cv;
    std::mutex m;
    std::atomic_bool exit = ATOMIC_VAR_INIT(false);

    kiwi::FiberPool* fiberPool = new kiwi::FiberPool(100);

    kiwi::SpinLock queueLock;
    kiwi::Queue<Jobs> queue(100);

    Jobs jobs[3];
    
    JobArg arg;
    arg.message = message;

    jobs[0].m_arg = &arg;
    jobs[0].m_function = GotHere;
    jobs[0].m_fiber = fiberPool->GetFiber();

    jobs[1].m_arg = nullptr;
    jobs[1].m_function = TheOtherHere;
    jobs[1].m_fiber = fiberPool->GetFiber();

    CounterTaskData counterTaskData;
    counterTaskData.fiberPool = fiberPool;
    counterTaskData.m_queueLock = &queueLock;
    counterTaskData.m_queue = &queue;
    counterTaskData.m_cv = &cv;

    jobs[2].m_arg = &counterTaskData;
    jobs[2].m_function = CounterTask;
    jobs[2].m_fiber = fiberPool->GetFiber();

    // JobsQueueNode node[2];
    // node[0].data.m_arg = &arg;
    // node[0].data.m_function = GotHere;
    // node[0].data.m_fiber = &fibers[0];

    // node[1].data.m_arg = nullptr;
    // node[1].data.m_function = TheOtherHere;
    // node[1].data.m_fiber = &fibers[1];

    // ck_spinlock_fas queueLock = CK_SPINLOCK_FAS_INITIALIZER;
    // ck_spinlock_fas_init(&queueLock);

    // ck_spinlock_fas_lock(&queueLock);
    // CK_STAILQ_INSERT_TAIL(&jobQueue, &node[0], list_entry);
    // ck_spinlock_fas_unlock(&queueLock);
    
    // ck_spinlock_fas_lock(&queueLock);
    // CK_STAILQ_INSERT_TAIL(&jobQueue, &node[1], list_entry);
    // ck_spinlock_fas_unlock(&queueLock);

    WorkerData workerData; //= new WorkerData();
    workerData.m_queue = &queue;
    workerData.m_queueLock = &queueLock;
    workerData.m_cv = &cv;
    workerData.m_m = &m;
    workerData.m_exit = &exit;
    workerData.m_fiberPool = fiberPool;

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

    queueLock.Lock();
    queue.Push(jobs[0]);
    queue.Push(jobs[1]);  
    queue.Push(jobs[2]);    
    queueLock.Unlock();

    cv.notify_all();

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

    sleep(2);

    exit.store(true);

    cv.notify_all();

    for (int32_t i = 0; i < cpuCount; ++i)
    {
        PTRD_ERR_HNDLR(pthread_join(workerThreads[i], NULL));
    }

    delete[] workerThreads;

    delete fiberPool;

    return 0;
}
