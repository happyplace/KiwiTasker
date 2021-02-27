#pragma once

#include <inttypes.h>

namespace kiwi
{
struct Fiber;

struct FiberWorkerStorage
{
    Fiber* m_fiber = nullptr;
};

void CreateFiberWorkerStorage(int cpuCount);
void DestroyFiberWorkerStorage();
FiberWorkerStorage* GetFiberWorkerStorage(int cpuIndex);
}
