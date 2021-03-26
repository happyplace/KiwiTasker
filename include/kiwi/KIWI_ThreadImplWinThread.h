#pragma once

#include <kiwi/KIWI_Std.h>

// reduce the amount of windows libraries linked
#define WIN32_LEAN_AND_MEAN 
#pragma comment(linker, "/subsystem:windows")

// needed to build on C
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0
#include <Windows.h>

#define WORKER_THREAD_DEFINITION(X) DWORD WINAPI SchedulerWorkerThread(LPVOID X)
#define WORKER_THREAD_RETURN_STATEMENT return 0

#ifdef __cplusplus
extern "C" {
#endif

struct KIWI_ThreadImpl;

// creates and starts workerCount number of worker threads and Returns ThreadImpl representing teh created worker threads
// KIWI_ThreadImplShutdownWorkerThreads needs to be called to shutdown worker threads and cleanup memory
struct KIWI_ThreadImpl* KIWI_ThreadImplCreateAndStartWorkerThreads(int workerCount, DWORD(*threadFunction) (LPVOID));

// shutdown worker threads an clean up memory
void KIWI_ThreadImplShutdownWorkerThreads(struct KIWI_ThreadImpl* threadImpl);

// this should be called by each worker thread to prevent receiving any signals
void KIWI_ThreadImplBlockSignalsOnWorkerThread();

// returns the physical count of cpus report by the system
int KIWI_ThreadImplGetCpuCount();

// returns the index of the worker.
// WARNING: this only works from kiwi worker threads, calling from other threads will have unexpected results
extern DECLSPEC int KIWI_ThreadImplGetWorkerThreadIndex();

#ifdef __cplusplus
}
#endif
