#pragma once

#include <inttypes.h>

#include "kiwi/Config.h"
#include "kiwi/Fiber.h"
#include "kiwi/SpinLock.h"

namespace kiwi
{
class FiberPool
{
public:
    FiberPool(int32_t poolSize);
    ~FiberPool();

    FiberPool(const FiberPool&) = delete;
    FiberPool& operator=(const FiberPool&) = delete;

    // get the next free fiber in the pool, or nullptr if there are no more fibers available
    Fiber* GetFiber();

    // return fiber to the pool
    void ReturnFiber(Fiber* fiber);

private:
    Fiber* CreatePool(int32_t poolSize) const;

    SpinLock m_lock;
    Fiber* m_pool = nullptr;    
    Fiber* m_firstFree = nullptr;   

#ifdef KIWI_FIBERPOOL_DYNAMIC_MODE
    struct FiberPoolLinks
    {
        Fiber* m_buffer = nullptr;
        FiberPoolLinks* m_next = nullptr;
    };
    FiberPoolLinks* m_firstPoolLink = nullptr;
    int32_t m_poolSize = 0;
#endif // KIWI_FIBERPOOL_DYNAMIC_MODE
};
}
