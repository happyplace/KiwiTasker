#pragma once

#include "kiwi/KIWI_std.h"
#include "kiwi/KIWI_JobPriority.h"

#ifdef __cplusplus
extern "C" {
#endif

struct KIWI_Scheduler;
struct KIWI_Job;

typedef struct KIWI_SchedulerParams
{
	// this is the max amount of jobs that can be in queue
	int jobQueueSize;

	// this is the amount of worker threads to create.
	// setting a value higher than the physical cpu count or a value lower than one will be ignored, and will instead use the phyiscal cpu count.
	// KIWI_DefaultSchedulerParams will set this value to the physical cpu count
	int workerCount;

	// this is the max amount of jobs you can have running or waiting on counters to complete
	int fiberPoolSize;
} KIWI_SchedulerParams;

// this will populate params with default values. These defaults can be used to further tweak options
extern DECLSPEC void KIWI_DefaultSchedulerParams(KIWI_SchedulerParams* params);

// creates KIWI_Scheduler, initializes memory and starts worker threads. This function should be called before calling 
// any other functions on the Scheduler.
// KIWI_FreeScheduler needs to be called to shutdown worker threads and free memory
extern DECLSPEC struct KIWI_Scheduler* KIWI_CreateScheduler(const KIWI_SchedulerParams* params);

// shuts down worker threads and cleans up memory for scheduler
extern DECLSPEC void KIWI_FreeScheduler(struct KIWI_Scheduler* scheduler);

// Adds the job to the job queue at priority level. A copy of the job is kept so it's safe to delete the job definition. Any memory used by the argument
// needs to be valid until the job completes.
extern DECLSPEC void KIWI_SchedulerAddJob(struct KIWI_Scheduler* scheduler, const struct KIWI_Job* job, const KIWI_JobPriority priority);

// Adds the jobs to the job queue at priority level. A copy of the job is kept so it's safe to delete the job definitions but any memory used by the argument 
// needs to be valid until the jobs completes.
extern DECLSPEC void KIWI_SchedulerAddJobs(struct KIWI_Scheduler* scheduler, const struct KIWI_Job* job, const int jobCount, const KIWI_JobPriority priority);

#ifdef __cplusplus
}
#endif
