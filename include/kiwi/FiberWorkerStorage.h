#pragma once

#include <bitset>

#include "kiwi/Fiber.h"

namespace kiwi
{
class Scheduler;
struct Fiber;

struct FiberWorkerData
{
    kiwi::Scheduler* m_scheduler = nullptr;
    kiwi::Context m_fiberWorkerReturn;
    kiwi::Fiber* m_currentFiber = nullptr;
    Fiber m_fiberPool[2];
    std::bitset<2> m_fiberPoolStatus;
    std::size_t m_fiberPoolIndex;
};
}
