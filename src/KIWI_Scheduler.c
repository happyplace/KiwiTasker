#include "kiwi/KIWI_Scheduler.h"

#include <malloc.h>

#include "kiwi/KIWI_std.h"

void KIWI_DefaultSchedulerParams(KIWI_SchedulerParams* params)
{
	KIWI_ASSERT(params);

	params->jobQueueSize = 1024;
}

void KIWI_InitializeScheduler(KIWI_Scheduler* scheduler, KIWI_SchedulerParams* params)
{
	KIWI_ASSERT(scheduler);

	(void)params;

	scheduler->impl = malloc(1024);
}

void KIWI_FreeScheduler(KIWI_Scheduler* scheduler)
{
	KIWI_ASSERT(scheduler);
	free(scheduler->impl);
}
