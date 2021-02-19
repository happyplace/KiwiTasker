#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "kiwi/Fiber.h"

namespace kiwi
{
class Scheduler;
struct Fiber;

// struct FiberWorkerData
// {
//     kiwi::Scheduler* m_scheduler = nullptr;
//     kiwi::Context m_fiberWorkerReturn;
//     kiwi::Fiber* m_currentFiber = nullptr;
//     Fiber m_fiberPool[2];
//     std::bitset<2> m_fiberPoolStatus;
//     std::size_t m_fiberPoolIndex;
// };

struct FiberWorkerStorage
{
    kiwi::Scheduler* m_scheduler = nullptr;
    std::condition_variable* m_conditionVariable = nullptr;
    std::mutex* m_mutex = nullptr;
    std::atomic_bool* m_closeWorker = nullptr;

    bool GetOrWaitForNextFiber(kiwi::Fiber** outFiber)
    {
        std::unique_lock<std::mutex> lock(*m_mutex);
        // if the workers are suppose to close just return nullptr
        // the worker thread will handle shuting down
        if (m_closeWorker->load())
        {
            return false;
        }
        m_conditionVariable->wait(lock);

        return false;
    }
};
}
