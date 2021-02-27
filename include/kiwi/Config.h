#pragma once

#include <inttypes.h>

namespace KiwiConfig
{
    // default size of job queues for the scheduler
    constexpr int32_t schedulerQueueSize = 1024;
    constexpr int32_t schdulerPendingFiberArraySize = 128;
    constexpr int32_t schedulerFiberPoolSize = 512;
    constexpr int32_t fiberStackSize = 64000;
}

// if this is defined fiber pools will dynamically create new pools if the pool gets full
//#ifndef KIWI_FIBERPOOL_DYNAMIC_MODE
//#define KIWI_FIBERPOOL_DYNAMIC_MODE
//#endif // KIWI_FIBERPOOL_DYNAMIC_MODE

// if this is defined queues will automatically expand their container size if they get filled
//#ifndef KIWI_QUEUE_DYNAMIC_MODE
//#define KIWI_QUEUE_DYNAMIC_MODE
//#endif // KIWI_QUEUE_DYNAMIC_MODE

// when queues are in dynamic mode this is how much to multiply the current container size when the queue is filled
//#ifndef KIWI_QUEUE_DYNAMIC_INCREASE_MULTIPLIER
//#define KIWI_QUEUE_DYNAMIC_INCREASE_MULTIPLIER 2.0f
//#endif // KIWI_QUEUE_DYNAMIC_INCREASE_MULTIPLIER

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)

// enables extra error checking for the scheduler
#ifndef KIWI_SCHEDULER_ERROR_CHECKING
#define KIWI_SCHEDULER_ERROR_CHECKING
#endif // KIWI_SCHEDULER_ERROR_CHECKING

// enables error checking for queues to makes sure invalid values aren't
// used for the size and detecting when the list is full
#ifndef KIWI_QUEUE_ERROR_CHECKING
#define KIWI_QUEUE_ERROR_CHECKING
#endif // KIWI_QUEUE_ERROR_CHECKING

// enables error checking for arrays. used for the size and detecting when the list is full
#ifndef KIWI_ARRAY_ERROR_CHECKING
#define KIWI_ARRAY_ERROR_CHECKING
#endif // KIWI_QUEUE_ERROR_CHECKING

// enables error checking for fiber pools, throws error if the fiber pool is ever full
#ifndef KIWI_FIBERPOOL_ERROR_CHECKING
#define KIWI_FIBERPOOL_ERROR_CHECKING
#endif // KIWI_FIBERPOOL_ERROR_CHECKING

#endif // defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
