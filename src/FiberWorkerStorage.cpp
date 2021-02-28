#include "kiwi/FiberWorkerStorage.h"

#include <assert.h>

#include "kiwi/Config.h"

using namespace kiwi;

static FiberWorkerStorage* s_fiberWorkerStorage = nullptr;
static int32_t s_fiberWorkerStorageCount = 0;

void kiwi::CreateFiberWorkerStorage(const int32_t cpuCount)
{
#ifdef KIWI_SCHEDULER_ERROR_CHECKING
    assert(s_fiberWorkerStorage == nullptr && "Fiber Storage already exists, did you call CreateFiberWorkerStorage more than once");
    assert(cpuCount > 0 && "why is there CPU count zero? It has to have atleast one");
#endif // KIWI_SCHEDULER_ERROR_CHECKING

    s_fiberWorkerStorageCount = cpuCount;
    s_fiberWorkerStorage = new FiberWorkerStorage[cpuCount];
}

void kiwi::DestroyFiberWorkerStorage()
{
#ifdef KIWI_SCHEDULER_ERROR_CHECKING
    assert(s_fiberWorkerStorage != nullptr && "Fiber storage was never created, are you missing a call to CreateFiberWorkerStorage?");
#endif // KIWI_SCHEDULER_ERROR_CHECKING
    delete[] s_fiberWorkerStorage;
    s_fiberWorkerStorage = nullptr;
}

FiberWorkerStorage* kiwi::GetFiberWorkerStorage(const int32_t cpuIndex)
{
#ifdef KIWI_SCHEDULER_ERROR_CHECKING
    assert(cpuIndex >= 0 && cpuIndex < s_fiberWorkerStorageCount && "attempting to use an invalid cpuIndex to get FiberWorkerStorage");
#endif // KIWI_SCHEDULER_ERROR_CHECKING
    return &s_fiberWorkerStorage[cpuIndex];
}
