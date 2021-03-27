#include "kiwi/KIWI_Scheduler.h"

#include <malloc.h>

#include "kiwi/KIWI_ThreadImpl.h"
#include "kiwi/KIWI_FiberWorkerStorage.h"
#include "kiwi/KIWI_Atomics.h"
#include "kiwi/KIWI_SpinLock.h"
#include "kiwi/KIWI_Queue.h"
#include "kiwi/KIWI_Job.h"
#include "kiwi/KIWI_FiberPool.h"

typedef struct KIWI_Scheduler
{
	struct KIWI_ThreadImpl* threadImpl;
	atomic_bool quitWorkerThreads;

	struct KIWI_SpinLock* queueLock;
	struct KIWI_Queue* queueLow;
	struct KIWI_Queue* queueNormal;
	struct KIWI_Queue* queueHigh;

	struct KIWI_FiberPool* fiberPool;
} KIWI_Scheduler;

typedef struct KIWI_PendingJob
{
	KIWI_Job job;
	// TODO: counter
} KIWI_PendingJob;

void KIWI_DefaultSchedulerParams(KIWI_SchedulerParams* params)
{
	KIWI_ASSERT(params);

	params->jobQueueSize = 1024;
	params->workerCount = KIWI_ThreadImplGetCpuCount();
	params->fiberPoolSize = 512;
}

bool KIWI_SchedulerGetNextFiber(KIWI_Scheduler* scheduler, KIWI_Fiber** outFiber, bool* outResumeFiber)
{
	(void)outFiber;

	(*outResumeFiber) = false;

	KIWI_LockSpinLock(scheduler->queueLock);

	if (!KIWI_QueueIsEmpty(scheduler->queueHigh))
	{
		KIWI_PendingJob pendingJob;
		bool result = KIWI_QueuePop(scheduler->queueHigh, &pendingJob);
		KIWI_ASSERT(result);	

		KIWI_Fiber* fiber = KIWI_FiberPoolGet(scheduler->fiberPool);
		(*outFiber) = fiber;
		fiber->job = pendingJob.job;

		KIWI_UnlockSpinLock(scheduler->queueLock);
		return true;		
	}
	else
	{
		KIWI_UnlockSpinLock(scheduler->queueLock);
		return false;
	}
}

WORKER_THREAD_DEFINITION(arg)
{
	KIWI_ALLOW_UNUSED(arg);

	KIWI_ThreadImplBlockSignalsOnWorkerThread();

	KIWI_FiberWorkerStorage* workerStorage = KIWI_GetFiberWorkerStorage(KIWI_ThreadImplGetWorkerThreadIndex());

	while (!atomic_load(workerStorage->quitWorkerThreads))
	{
		KIWI_Fiber* fiber = NULL;
		bool resumeFiber = false;
		if (KIWI_SchedulerGetNextFiber(workerStorage->scheduler, &fiber, &resumeFiber))
		{
			fiber->job.entry(workerStorage->scheduler, fiber->job.arg);
			KIWI_FiberPoolReturn(workerStorage->scheduler->fiberPool, fiber);
		}

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

	scheduler->queueLock = KIWI_CreateSpinLock();
	if (scheduler->queueLock == NULL)
	{
		KIWI_ASSERT(!"are we out of memory???");
		return NULL;
	}

	scheduler->queueLow = KIWI_CreateQueue(sizeof(KIWI_PendingJob), params->jobQueueSize);
	if (scheduler->queueLow == NULL)
	{
		KIWI_ASSERT(!"are we out of memory???");
		return NULL;
	}

	scheduler->queueNormal = KIWI_CreateQueue(sizeof(KIWI_PendingJob), params->jobQueueSize);
	if (scheduler->queueNormal == NULL)
	{
		KIWI_ASSERT(!"are we out of memory???");
		return NULL;
	}

	scheduler->queueHigh = KIWI_CreateQueue(sizeof(KIWI_PendingJob), params->jobQueueSize);
	if (scheduler->queueHigh == NULL)
	{
		KIWI_ASSERT(!"are we out of memory???");
		return NULL;
	}

	int cpuCount = KIWI_ThreadImplGetCpuCount();
	if (params->workerCount < cpuCount && params->workerCount >= 1)
	{
		cpuCount = params->workerCount;
	}
	
	scheduler->fiberPool = KIWI_CreateFiberPool(params->fiberPoolSize);

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

	KIWI_FreeSpinLock(scheduler->queueLock);
	KIWI_FreeQueue(scheduler->queueLow);
	KIWI_FreeQueue(scheduler->queueNormal);
	KIWI_FreeQueue(scheduler->queueHigh);

	KIWI_FreeFiberPool(scheduler->fiberPool);

	free(scheduler);
}
