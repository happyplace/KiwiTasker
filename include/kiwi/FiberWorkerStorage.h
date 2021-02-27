#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "kiwi/Context.h"
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
    Scheduler* m_scheduler = nullptr;
    Context m_context;

    std::condition_variable* m_conditionVariable = nullptr;
    std::mutex* m_mutex = nullptr;
    std::atomic_bool* m_closeWorker = nullptr;

    FiberPool* m_fiberPool = nullptr;
    Fiber* m_fiber = nullptr;

    SpinLock* m_queueLock = nullptr;    
    Queue<PendingJob>* m_queueHigh = nullptr;
    Queue<PendingJob>* m_queueNormal = nullptr;    
    Queue<PendingJob>* m_queueLow = nullptr;

    bool GetOrWaitForNextFiber(kiwi::Fiber** outFiber);
};
}

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
    kiwi::Fiber* m_currentFiber = nullptr;
};

void CreateWorkerData(int cpuCount);
void DestroyWorkerData();
WorkerData* GetWorkerData(int cpuIndex);
