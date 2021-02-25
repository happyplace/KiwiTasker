#include "kiwi/FiberWorkerThreadMain.h"

#include "kiwi/Scheduler.h"
#include "kiwi/SchedulerImpl.h"
#include "kiwi/FiberWorkerStorage.h"

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

void FiberStart(Scheduler* scheduler, kiwi::Fiber* fiber)
{
    fiber->m_job.m_function(scheduler, fiber->m_job.m_arg);

    // return fiber to pool
    scheduler->GetFiberWorkerStorage()->m_fiberPool->ReturnFiber(fiber);

    // return to fiber worker main function
    set_context(&scheduler->GetFiberWorkerStorage()->m_context);
    
    assert(!"should never get here");
}

void* kiwi::FiberWorkerThreadMain(void* arg)
{
    Scheduler* scheduler = reinterpret_cast<Scheduler*>(arg);
    SchedulerImpl* schedulerImpl = scheduler->GetImpl();
    
    schedulerImpl->BlockSignalsOnThread();

    FiberWorkerStorage* storage = scheduler->GetFiberWorkerStorage();

    get_context(&storage->m_context);

    while (!storage->m_closeWorker->load())
    {                  
        Fiber* fiber = nullptr;  
        if (storage->GetOrWaitForNextFiber(&fiber))
        {
            //char* stackPointer = schedulerImpl->GetStackPointerForStackBuffer(fiber->m_stack);
            //schedulerImpl->SetContextInstructionAndStack(&fiber->m_context, (void*)FiberStart, (void*)stackPointer);
            //schedulerImpl->SetContextParameters(&fiber->m_context, scheduler, fiber, nullptr);
            
            //set_context(&fiber->m_context); 

            assert(!"should never get here");
        }
    }

    return 0;
}
