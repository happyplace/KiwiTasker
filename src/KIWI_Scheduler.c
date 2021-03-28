#include "kiwi/KIWI_Scheduler.h"

#include <malloc.h>

#include "kiwi/KIWI_ThreadImpl.h"
#include "kiwi/KIWI_FiberWorkerStorage.h"
#include "kiwi/KIWI_Atomics.h"
#include "kiwi/KIWI_SpinLock.h"
#include "kiwi/KIWI_Queue.h"
#include "kiwi/KIWI_Job.h"
#include "kiwi/KIWI_FiberPool.h"
#include "kiwi/KIWI_Counter.h"
#include "kiwi/KIWI_Array.h"

typedef struct KIWI_Scheduler
{
	struct KIWI_ThreadImpl* threadImpl;
	atomic_bool quitWorkerThreads;

	struct KIWI_SpinLock* queueLock;
	struct KIWI_Queue* queueLow;
	struct KIWI_Queue* queueNormal;
	struct KIWI_Queue* queueHigh;
	struct KIWI_Queue* queueReadyFibers;

	struct KIWI_SpinLock* waitingFibersLock;
	struct KIWI_Array* waitingFibers;

	struct KIWI_FiberPool* fiberPool;

	struct KIWI_SpinLock* counterLock;
	struct KIWI_Array* counters;
} KIWI_Scheduler;

typedef struct KIWI_CounterContainer
{
	struct KIWI_Counter* counter;
} KIWI_CounterContainer;

typedef struct KIWI_PendingJob
{
	KIWI_Job job;
	struct KIWI_CounterContainer counter;
} KIWI_PendingJob;

typedef struct KIWI_WaitingFiber
{
	struct KIWI_Fiber* fiber;
	struct KIWI_Counter* counter;
	int targetValue;
} KIWI_WaitingFiber;

void KIWI_DefaultSchedulerParams(KIWI_SchedulerParams* params)
{
	KIWI_ASSERT(params);

	params->jobQueueSize = 1024;
	params->workerCount = KIWI_ThreadImplGetCpuCount();
	params->fiberPoolSize = 512;
	params->fiberStackSize = 512 * 1024;
	params->countersCapacity = 25;
	params->waitingFiberCapacity = 128;
}

bool KIWI_SchedulerGetNextFiber(KIWI_Scheduler* scheduler, KIWI_Fiber** outFiber, bool* outResumeFiber)
{
	(*outResumeFiber) = false;

	KIWI_LockSpinLock(scheduler->queueLock);

	if (!KIWI_QueueIsEmpty(scheduler->queueHigh))
	{
		KIWI_PendingJob pendingJob;
		bool result = KIWI_QueuePop(scheduler->queueHigh, &pendingJob);
		KIWI_ASSERT(result);
		KIWI_ALLOW_UNUSED(result);

		KIWI_Fiber* fiber = KIWI_FiberPoolGet(scheduler->fiberPool);
		(*outFiber) = fiber;
		fiber->job = pendingJob.job;
		fiber->counter = pendingJob.counter.counter;

		KIWI_UnlockSpinLock(scheduler->queueLock);
		return true;		
	}
	else if (!KIWI_QueueIsEmpty(scheduler->queueReadyFibers))
	{
		KIWI_Fiber* fiber = NULL;
		bool result = KIWI_QueuePop(scheduler->queueReadyFibers, &fiber);
		KIWI_ASSERT(result);
		KIWI_ALLOW_UNUSED(result);

		(*outResumeFiber) = true;
		(*outFiber) = fiber;

		KIWI_UnlockSpinLock(scheduler->queueLock);
		return true;
	}
	else if (!KIWI_QueueIsEmpty(scheduler->queueNormal))
	{
		KIWI_PendingJob pendingJob;
		bool result = KIWI_QueuePop(scheduler->queueNormal, &pendingJob);
		KIWI_ASSERT(result);
		KIWI_ALLOW_UNUSED(result);

		KIWI_Fiber* fiber = KIWI_FiberPoolGet(scheduler->fiberPool);
		(*outFiber) = fiber;
		fiber->job = pendingJob.job;
		fiber->counter = pendingJob.counter.counter;

		KIWI_UnlockSpinLock(scheduler->queueLock);
		return true;
	}
	else if (!KIWI_QueueIsEmpty(scheduler->queueLow))
	{
		KIWI_PendingJob pendingJob;
		bool result = KIWI_QueuePop(scheduler->queueLow, &pendingJob);
		KIWI_ASSERT(result);
		KIWI_ALLOW_UNUSED(result);

		KIWI_Fiber* fiber = KIWI_FiberPoolGet(scheduler->fiberPool);
		(*outFiber) = fiber;
		fiber->job = pendingJob.job;
		fiber->counter = pendingJob.counter.counter;

		KIWI_UnlockSpinLock(scheduler->queueLock);
		return true;
	}
	else
	{
		KIWI_UnlockSpinLock(scheduler->queueLock);
		return false;
	}
}

void KIWI_FiberEntry(fcontext_transfer_t t)
{	
	KIWI_FiberWorkerStorage* workerStorage = KIWI_GetFiberWorkerStorage(KIWI_ThreadImplGetWorkerThreadIndex());
	KIWI_Fiber* fiber = workerStorage->fiber;
	
	fiber->context = t;

	fiber->job.entry(workerStorage->scheduler, fiber->job.arg);
	
	// updating worker storage and fiber because we could be on another thread now
	workerStorage = KIWI_GetFiberWorkerStorage(KIWI_ThreadImplGetWorkerThreadIndex());
	fiber = workerStorage->fiber;

	workerStorage->completedFiber = true;
	jump_fcontext(fiber->context.ctx, NULL);
}

void KIWI_AddFiberToReadyQueue(KIWI_Scheduler* scheduler, KIWI_Fiber* fiber)
{
	KIWI_LockSpinLock(scheduler->queueLock);
	KIWI_QueuePush(scheduler->queueReadyFibers, &fiber);
	KIWI_UnlockSpinLock(scheduler->queueLock);

	KIWI_ThreadImplNotifyOneWorkerThread(scheduler->threadImpl);
}

void KIWI_CheckWaitingFibers(KIWI_Scheduler* scheduler, struct KIWI_Counter* counter, int value)
{
	KIWI_LockSpinLock(scheduler->waitingFibersLock);
	int size = KIWI_ArraySize(scheduler->waitingFibers);
	for (int i = 0; i < size; ++i)
	{
		KIWI_WaitingFiber* waitingFiber = KIWI_ArrayGet(scheduler->waitingFibers, i);
		if (waitingFiber->counter == counter)
		{
			if (waitingFiber->targetValue == value)
			{
				KIWI_ArrayRemoveItem(scheduler->waitingFibers, i);
				i--;
				size--;

				KIWI_AddFiberToReadyQueue(scheduler, waitingFiber->fiber);
			}
		}
	}
	KIWI_UnlockSpinLock(scheduler->waitingFibersLock);
}

WORKER_THREAD_DEFINITION(arg)
{
	KIWI_ALLOW_UNUSED(arg);

	KIWI_ThreadImplBlockSignalsOnWorkerThread();

	KIWI_FiberWorkerStorage* workerStorage = KIWI_GetFiberWorkerStorage(KIWI_ThreadImplGetWorkerThreadIndex());

	while (!atomic_load(KIWI_GetFiberWorkerStorage(KIWI_ThreadImplGetWorkerThreadIndex())->quitWorkerThreads))
	{
		KIWI_Fiber* fiber = NULL;
		bool resumeFiber = false;
		if (KIWI_SchedulerGetNextFiber(workerStorage->scheduler, &fiber, &resumeFiber))
		{
			workerStorage->fiber = fiber;
			
			if (resumeFiber)
			{
				// we're going back in
				fiber->context = jump_fcontext(fiber->context.ctx, NULL);
			}
			else
			{
				// this is a new fiber, we need to create new context 
				fcontext_t fiberEntry = make_fcontext(fiber->stack.sptr, fiber->stack.ssize, KIWI_FiberEntry);

				// make the jump, see you on the other side
				fiber->context = jump_fcontext(fiberEntry, NULL);
			}

			// we could be returning on a different thread, refresh worker storage and fiber
			workerStorage = KIWI_GetFiberWorkerStorage(KIWI_ThreadImplGetWorkerThreadIndex());
			fiber = workerStorage->fiber;

			// if we're coming back here we need to check if the fiber actually completed
			// if it didn't it's already been added to the waiting queue so we just do nothing and look for the next job
			if (workerStorage->completedFiber)
			{
				if (fiber->counter)
				{
					int value = KIWI_DecrementCounter(fiber->counter);
					KIWI_CheckWaitingFibers(workerStorage->scheduler, fiber->counter, value);
				}

				KIWI_FiberPoolReturn(workerStorage->scheduler->fiberPool, fiber);
			}
			else
			{
				// we can release the counter lock now because the fiber can be successfully resumed
				struct KIWI_Counter* counter = fiber->context.data;
				KIWI_CounterUnlock(counter);
			}
		}
		else
		{	
			workerStorage = KIWI_GetFiberWorkerStorage(KIWI_ThreadImplGetWorkerThreadIndex());
			KIWI_ThreadImplSleepUntilJobAdded(workerStorage->threadImpl, workerStorage->quitWorkerThreads);
		}
	}

	WORKER_THREAD_RETURN_STATEMENT;
}

KIWI_Scheduler* KIWI_CreateScheduler(const KIWI_SchedulerParams* params)
{
	KIWI_Scheduler* scheduler = malloc(sizeof(KIWI_Scheduler));
	if (scheduler == NULL)
	{
		KIWI_ASSERT(!"are we out of memory???");
		return NULL;
	}

	scheduler->queueLock = KIWI_CreateSpinLock();

	scheduler->queueLow = KIWI_CreateQueue(sizeof(KIWI_PendingJob), params->jobQueueSize);
	scheduler->queueNormal = KIWI_CreateQueue(sizeof(KIWI_PendingJob), params->jobQueueSize);
	scheduler->queueHigh = KIWI_CreateQueue(sizeof(KIWI_PendingJob), params->jobQueueSize);

	scheduler->queueReadyFibers = KIWI_CreateQueue(sizeof(KIWI_Fiber*), params->waitingFiberCapacity);
	
	scheduler->waitingFibersLock = KIWI_CreateSpinLock();
	scheduler->waitingFibers = KIWI_CreateArray(sizeof(KIWI_WaitingFiber), params->waitingFiberCapacity);

	int cpuCount = KIWI_ThreadImplGetCpuCount();
	if (params->workerCount < cpuCount && params->workerCount >= 1)
	{
		cpuCount = params->workerCount;
	}
	
	scheduler->fiberPool = KIWI_CreateFiberPool(params->fiberPoolSize, params->fiberStackSize);

	scheduler->counterLock = KIWI_CreateSpinLock();
	scheduler->counters = KIWI_CreateArray(sizeof(struct KIWI_CounterContainer), params->countersCapacity);

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
	KIWI_FreeQueue(scheduler->queueReadyFibers);

	KIWI_FreeSpinLock(scheduler->waitingFibersLock);
	KIWI_FreeArray(scheduler->waitingFibers);

	KIWI_FreeFiberPool(scheduler->fiberPool);

	KIWI_FreeSpinLock(scheduler->counterLock);
	KIWI_FreeArray(scheduler->counters);

	free(scheduler);
}

void KIWI_SchedulerAddJob(struct KIWI_Scheduler* scheduler, const struct KIWI_Job* job, const KIWI_JobPriority priority, struct KIWI_Counter** counter)
{
	KIWI_ASSERT(scheduler);
	KIWI_ASSERT(job);

	KIWI_SchedulerAddJobs(scheduler, job, 1, priority, counter);
}

struct KIWI_Counter* KIWI_SchedulerCreateCounter(struct KIWI_Scheduler* scheduler)
{
	KIWI_ASSERT(scheduler);

	struct KIWI_Counter* counter = KIWI_CreateCounter();
	KIWI_LockSpinLock(scheduler->counterLock);
	KIWI_ArrayAddItem(scheduler->counters, &counter);
	KIWI_UnlockSpinLock(scheduler->counterLock);
	return counter;
}

void KIWI_SchedulerFreeCounter(struct KIWI_Scheduler* scheduler, struct KIWI_Counter* counter)
{
	KIWI_ASSERT(scheduler);
	KIWI_ASSERT(counter);

	KIWI_LockSpinLock(scheduler->counterLock);
	int size = KIWI_ArraySize(scheduler->counters);
	for (int i = 0; i < size; ++i)
	{
		KIWI_CounterContainer* container = KIWI_ArrayGet(scheduler->counters, i);
		if (container->counter == counter)
		{
			KIWI_ArrayRemoveItem(scheduler->counters, i);
			KIWI_FreeCounter(counter);
		}
	}

	KIWI_ArrayAddItem(scheduler->counters, &counter);
	KIWI_UnlockSpinLock(scheduler->counterLock);
}

void KIWI_SchedulerAddJobs(struct KIWI_Scheduler* scheduler, const struct KIWI_Job* job, const int jobCount, const KIWI_JobPriority priority, struct KIWI_Counter** outCounter)
{	
	KIWI_ASSERT(scheduler);
	KIWI_ASSERT(job);
	KIWI_ASSERT(jobCount >= 1);

	struct KIWI_Queue* queue = NULL;
	switch (priority)
	{
	case KIWI_JobPriority_High:
		queue = scheduler->queueHigh;
		break;
	case KIWI_JobPriority_Normal:
		queue = scheduler->queueNormal;
		break;
	case KIWI_JobPriority_Low:
		queue = scheduler->queueLow;
		break;
	default:
		KIWI_ASSERT(!"Unknown job priority, normal priority will be used");
		queue = scheduler->queueNormal;
		break;
	}

	struct KIWI_Counter* counter = NULL;
	if (outCounter != NULL)
	{
		if ((*outCounter) != NULL)
		{
			// they're reusing a counter so we don't need to create one
			counter = (*outCounter);
		}
		else
		{
			counter = KIWI_SchedulerCreateCounter(scheduler);
			(*outCounter) = counter;
		}
	}

	KIWI_LockSpinLock(scheduler->queueLock);
	for (int i = 0; i < jobCount; ++i)
	{
		KIWI_PendingJob pendingJob;
		memcpy(&pendingJob.job, ((char*)job + (sizeof(KIWI_Job) * i)), sizeof(KIWI_Job));
		pendingJob.counter.counter = counter;
		KIWI_QueuePush(queue, &pendingJob);
		
		if (counter)
		{
			KIWI_IncrementCounter(counter);
		}
	}
	KIWI_UnlockSpinLock(scheduler->queueLock);

	if (jobCount == 1)
	{
		KIWI_ThreadImplNotifyOneWorkerThread(scheduler->threadImpl);
	}
	else
	{
		KIWI_ThreadImplNotifyAllWorkerThreads(scheduler->threadImpl);
	}
}

void KIWI_SchedulerWaitForCounterAndFree(struct KIWI_Scheduler* scheduler, struct KIWI_Counter* counter, int targetValue)
{
	KIWI_SchedulerWaitForCounter(scheduler, counter, targetValue);

	// when we return the jobs should be complete so we can resume
	KIWI_SchedulerFreeCounter(scheduler, counter);
}

void KIWI_SchedulerWaitForCounter(struct KIWI_Scheduler* scheduler, struct KIWI_Counter* counter, int targetValue)
{
	KIWI_ASSERT(scheduler);
	KIWI_ASSERT(counter);
	KIWI_ASSERT(targetValue >= 0);

	KIWI_FiberWorkerStorage* workerStorage = KIWI_GetFiberWorkerStorage(KIWI_ThreadImplGetWorkerThreadIndex());

	KIWI_WaitingFiber waitingFiber;
	waitingFiber.fiber = workerStorage->fiber;
	waitingFiber.targetValue = targetValue;
	waitingFiber.counter = counter;

	int counterValue = KIWI_CounterLockAndGetValue(counter);
	if (counterValue == targetValue)
	{
		KIWI_CounterUnlock(counter);
		return;
	}
	
	KIWI_LockSpinLock(scheduler->waitingFibersLock);
	KIWI_ArrayAddItem(scheduler->waitingFibers, &waitingFiber);
	KIWI_UnlockSpinLock(scheduler->waitingFibersLock);

	workerStorage->completedFiber = false;

	// because the fiber's context is pointing at the context to return here we can't unlock the counter
	// so we send the counter so we can unlock it when we get back to the worker thread main function.
	fcontext_transfer_t returnTransfer = jump_fcontext(workerStorage->fiber->context.ctx, counter);

	workerStorage = KIWI_GetFiberWorkerStorage(KIWI_ThreadImplGetWorkerThreadIndex());
	workerStorage->fiber->context = returnTransfer;
}
