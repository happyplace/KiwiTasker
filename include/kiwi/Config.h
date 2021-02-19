#pragma once

#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)

// enables extra error checking one queues to makes sure invalid values aren't
// used for the size and detecting when the list is full
#ifndef KIWI_QUEUE_ERROR_CHECKING
#define KIWI_QUEUE_ERROR_CHECKING
#endif // KIWI_QUEUE_ERROR_CHECKING

// if this is defined queues will automatically expand their container size if they get filled
//#ifndef KIWI_QUEUE_DYNAMIC_MODE
//#define KIWI_QUEUE_DYNAMIC_MODE
//#endif // KIWI_QUEUE_DYNAMIC_MODE

// when queues are in dynamic mode this is how much to multiply the current container size when the queue is filled
//#ifndef KIWI_QUEUE_DYNAMIC_INCREASE_MULTIPLIER
//#define KIWI_QUEUE_DYNAMIC_INCREASE_MULTIPLIER 2.0f
//#endif // KIWI_QUEUE_DYNAMIC_INCREASE_MULTIPLIER

#endif // defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
