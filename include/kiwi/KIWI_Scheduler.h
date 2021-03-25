#pragma once

#include "kiwi/KIWI_std.h"

#ifdef __cplusplus
extern "C" {
#endif

struct KIWI_Scheduler;

typedef struct KIWI_SchedulerParams
{
	// this is the max amount of jobs that can be in queue
	int jobQueueSize;
} KIWI_SchedulerParams;

// this will populate params with default values. These defaults can be used to further tweak options
extern DECLSPEC void KIWI_DefaultSchedulerParams(KIWI_SchedulerParams* params);

// creates KIWI_Scheduler, initializes memory and starts worker threads. This function should be called before calling 
// any other functions on the Scheduler.
// KIWI_FreeScheduler needs to be called to shutdown worker threads and free memory
extern DECLSPEC struct KIWI_Scheduler* KIWI_InitializeScheduler(KIWI_SchedulerParams* params);

// shuts down worker threads and cleans up memory for scheduler
extern DECLSPEC void KIWI_FreeScheduler(struct KIWI_Scheduler* scheduler);

#ifdef __cplusplus
}
#endif
