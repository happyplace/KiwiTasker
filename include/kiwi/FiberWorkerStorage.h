#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "kiwi/FiberPool.h"
#include "kiwi/PendingJob.h"
#include "kiwi/Queue.h"
#include "kiwi/SpinLock.h"

namespace kiwi
{
class Scheduler;
struct Fiber;

struct FiberWorkerStorage
{
    kiwi::Scheduler* m_scheduler = nullptr;
    std::condition_variable* m_conditionVariable = nullptr;
    std::mutex* m_mutex = nullptr;
    std::atomic_bool* m_closeWorker = nullptr;

    FiberPool* m_fiberPool = nullptr;

    SpinLock* m_queueLock = nullptr;    
    Queue<PendingJob>* m_queueHigh = nullptr;
    Queue<PendingJob>* m_queueNormal = nullptr;    
    Queue<PendingJob>* m_queueLow = nullptr;

    bool GetOrWaitForNextFiber(kiwi::Fiber** outFiber);
};
}
