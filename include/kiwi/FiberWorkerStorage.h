#pragma once

#include <inttypes.h>

#include "kiwi/Context.h"

namespace kiwi
{
struct Fiber;

struct FiberWorkerStorage
{
    Fiber* m_fiber = nullptr;
    Context m_context = {0};
};

void CreateFiberWorkerStorage(int cpuCount);
void DestroyFiberWorkerStorage();
FiberWorkerStorage* GetFiberWorkerStorage(int cpuIndex);
}
