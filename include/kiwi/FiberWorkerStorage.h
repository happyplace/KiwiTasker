#pragma once

#include <inttypes.h>

namespace kiwi
{
class Scheduler;
struct Fiber;

struct FiberWorkerStorage
{
};

void CreateFiberWorkerStorage(int cpuCount);
void DestroyFiberWorkerStorage();
FiberWorkerStorage* GetFiberWorkerStorage(int cpuIndex);
}
