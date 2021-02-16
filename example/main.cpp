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

volatile int x = 2;
kiwi::ContextUnix retMain;

std::queue<int> pendingTasks;

struct BoyData
{
    const char* m_secretMsg = "This is the super secret message";
    int fam = 0;
};

void DatFunctionBoy(kiwi::Scheduler* scheduler, void* data)
{
    int datData = *(reinterpret_cast<int*>(data));
    printf("%i)\n", datData);  
}

void fiber_start(void* scheduler, void* returnContext, void* arg)
{
    DatFunctionBoy(NULL, arg);

    // return to fiber worker after executing thread
    set_context(reinterpret_cast<kiwi::ContextUnix*>(returnContext));
}

static void* start_fiberWorker(void* arg)
{
    kiwi::Scheduler scheduler;

    kiwi::ContextUnix fiberWorkerReturn;
    get_context(&fiberWorkerReturn);
    fiberWorkerReturn.rdi = arg;

    while (!pendingTasks.empty())
    {
        int number = pendingTasks.front();
        pendingTasks.pop();
        
        char data[4096];

        char *sp = (char*)(data + sizeof(data));

        // Align stack pointer on 16-byte boundary.
        sp = (char*)((uintptr_t)sp & -16L);

        // Make 128 byte scratch space for the Red Zone. This arithmetic will not unalign
        // our stack pointer because 128 is a multiple of 16. The Red Zone must also be
        // 16-byte aligned.
        sp -= 128;

        kiwi::ContextUnix c = {0};
        c.rip = (void*)fiber_start;
        c.rsp = (void*)sp;
        c.rdi = &scheduler;
        c.rsi = &fiberWorkerReturn;
        c.rdx = &number;
        set_context(&c);
    }
    return NULL;
}

#include <unistd.h>

int main(int argc, char** argv)
{
    kiwi::Scheduler scheduler;

    pendingTasks.push(13);
    pendingTasks.push(99);
    pendingTasks.push(23);

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

    cpu_set_t cpuset;      
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    result = pthread_setaffinity_np(worker, sizeof(cpuset), &cpuset);
    if (result != 0)
    {
        assert(false);
        return 1;
    }

    while (!pendingTasks.empty())
    {
        sleep(1);
    }

    return 0;
}
