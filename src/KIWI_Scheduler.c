#include "kiwi/KIWI_Scheduler.h"

#include <malloc.h>

#include "kiwi/KIWI_std.h"

typedef struct KIWI_Scheduler
{
	int dummy;
} KIWI_Scheduler;

void KIWI_DefaultSchedulerParams(KIWI_SchedulerParams* params)
{
	KIWI_ASSERT(params);

	params->jobQueueSize = 1024;
}

KIWI_Scheduler* KIWI_CreateScheduler(KIWI_SchedulerParams* params)
{
	(void)params;

	KIWI_Scheduler* scheduler = malloc(sizeof(KIWI_Scheduler));
	return scheduler;

	//scheduler->entry(scheduler, scheduler->arg);
}

void KIWI_FreeScheduler(KIWI_Scheduler* scheduler)
{
	KIWI_ASSERT(scheduler);
	free(scheduler);
}
