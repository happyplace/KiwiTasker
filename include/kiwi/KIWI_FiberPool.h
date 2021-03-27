#pragma once

#include "kiwi/KIWI_Job.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KIWI_Fiber
{  
    KIWI_Job job;
    struct KIWI_Fiber* next;
} KIWI_Fiber;

struct KIWI_FiberPool;

// creates a pool of fibers of size poolSize. The Object returned by this function needs to be freed with KIWI_FreeFiberPool
struct KIWI_FiberPool* KIWI_CreateFiberPool(int poolSize);

// frees all memory associated with a fiber pool
void KIWI_FreeFiberPool(struct KIWI_FiberPool* fiberPool);

// returns the next free fiber in the pool
KIWI_Fiber* KIWI_FiberPoolGet(struct KIWI_FiberPool* fiberPool);

// returns fiber to pool
void KIWI_FiberPoolReturn(struct KIWI_FiberPool* fiberPool, KIWI_Fiber* fiber);

#ifdef __cplusplus
}
#endif
