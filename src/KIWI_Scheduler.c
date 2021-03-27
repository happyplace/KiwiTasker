#include "kiwi/KIWI_Scheduler.h"

#include <malloc.h>

#include "kiwi/KIWI_ThreadImpl.h"
#include "kiwi/KIWI_FiberWorkerStorage.h"
#include "kiwi/KIWI_Atomics.h"

typedef struct KIWI_Scheduler
{
	struct KIWI_ThreadImpl* threadImpl;
	atomic_bool quitWorkerThreads;
} KIWI_Scheduler;

void KIWI_DefaultSchedulerParams(KIWI_SchedulerParams* params)
{
	KIWI_ASSERT(params);

	params->jobQueueSize = 1024;
	params->workerCount = KIWI_ThreadImplGetCpuCount();
}

WORKER_THREAD_DEFINITION(arg)
{
	KIWI_ALLOW_UNUSED(arg);

	KIWI_ThreadImplBlockSignalsOnWorkerThread();

	KIWI_FiberWorkerStorage* workerStorage = KIWI_GetFiberWorkerStorage(KIWI_ThreadImplGetWorkerThreadIndex());

	while (!atomic_load(workerStorage->quitWorkerThreads))
	{
		KIWI_ThreadImplSleepUntilJobAdded(workerStorage->threadImpl, workerStorage->quitWorkerThreads);
	}

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
	
	KIWI_CreateFiberWorkerStorage(cpuCount);
	for (int i = 0; i < cpuCount; ++i)
	{
		KIWI_FiberWorkerStorage* workerStorage = KIWI_GetFiberWorkerStorage(i);
		workerStorage->quitWorkerThreads = &scheduler->quitWorkerThreads;
		workerStorage->scheduler = scheduler;
	}

	atomic_store(&scheduler->quitWorkerThreads, false);
	scheduler->threadImpl = KIWI_ThreadImplCreateAndStartWorkerThreads(cpuCount, SchedulerWorkerThread);

	return scheduler;
}

void KIWI_FreeScheduler(KIWI_Scheduler* scheduler)
{
	KIWI_ASSERT(scheduler);

	KIWI_ThreadImplSignalWorkerThreadsToQuit(scheduler->threadImpl, &scheduler->quitWorkerThreads);
	KIWI_ThreadImplNotifyAllWorkerThreads(scheduler->threadImpl);
	KIWI_ThreadImplShutdownWorkerThreads(scheduler->threadImpl);

	KIWI_DestroyFiberWorkerStorage();

	free(scheduler);
}
