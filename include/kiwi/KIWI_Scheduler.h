#pragma once

#include "kiwi/KIWI_std.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KIWI_Scheduler
{
	void* impl;
} KIWI_Scheduler;

typedef struct KIWI_SchedulerParams
{
	// this is the max amount of jobs that can be in the queue at one time
	int jobQueueSize;
} KIWI_SchedulerParams;

// this will populate params with default values, you can use this to further tweak options
extern DECLSPEC void KIWI_DefaultSchedulerParams(KIWI_SchedulerParams* params);

// initializes memory and starts worker threads, this function should be called before calling 
// any other functions on the Scheduler.
// KIWI_FreeScheduler needs to be called to shutdown worker threads and free memory
extern DECLSPEC void KIWI_InitializeScheduler(KIWI_Scheduler* scheduler, KIWI_SchedulerParams* params);

// shuts down worker threads and cleans up memory for scheduler
extern DECLSPEC void KIWI_FreeScheduler(KIWI_Scheduler* scheduler);

#ifdef __cplusplus
}
#endif
