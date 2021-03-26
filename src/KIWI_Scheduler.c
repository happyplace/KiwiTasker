#include "kiwi/KIWI_Scheduler.h"

#include <malloc.h>

#include "kiwi/KIWI_ThreadImpl.h"

typedef struct KIWI_Scheduler
{
	struct KIWI_ThreadImpl* threadImpl;
} KIWI_Scheduler;

void KIWI_DefaultSchedulerParams(KIWI_SchedulerParams* params)
{
	KIWI_ASSERT(params);

	params->jobQueueSize = 1024;
	params->workerCount = KIWI_ThreadImplGetCpuCount();
}

#include <stdio.h>

WORKER_THREAD_DEFINITION(arg)
{
	(void)arg;

	KIWI_ThreadImplBlockSignalsOnWorkerThread();

	printf("started worker thread\n");

	WORKER_THREAD_RETURN_STATEMENT;
}

KIWI_Scheduler* KIWI_CreateScheduler(KIWI_SchedulerParams* params)
{
	KIWI_Scheduler* scheduler = malloc(sizeof(KIWI_Scheduler));
	if (scheduler == NULL)
	{
		KIWI_ASSERT(!"are we out of memory???");
		return NULL;
	}

	int cpuCount = KIWI_ThreadImplGetCpuCount();
	if (params->workerCount < cpuCount && params->workerCount >= 1)
	{
		cpuCount = params->workerCount;
	}
	
	scheduler->threadImpl = KIWI_ThreadImplCreateAndStartWorkerThreads(cpuCount, SchedulerWorkerThread);

	return scheduler;
}

void KIWI_FreeScheduler(KIWI_Scheduler* scheduler)
{
	KIWI_ASSERT(scheduler);

	KIWI_ThreadImplShutdownWorkerThreads(scheduler->threadImpl);

	free(scheduler);
}
