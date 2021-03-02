#pragma once

#include <inttypes.h>

#include <Windows.h>

#define WORKER_THREAD_DEFINITION(X) DWORD WINAPI SchedulerWorkerThread(LPVOID X)
#define WORKER_THREAD_RETURN_STATEMENT return 0

namespace kiwi
{
    class SchedulerThreadImpl
    {
    public:
        SchedulerThreadImpl();
        ~SchedulerThreadImpl();

        SchedulerThreadImpl(const SchedulerThreadImpl&) = delete;
        SchedulerThreadImpl& operator=(const SchedulerThreadImpl&) = delete;

        void CreateThread(const char* threadName, int32_t threadAffinity, DWORD (*threadFunction) (LPVOID), void* threadFunctionArg);

        // this should be called by each worker thread to prevent receiving any signals
        void BlockSignalsOnWorkerThread() const;

        void JoinWorkerThreads();

        // this uses the thread affinity to determine what the thread index is
        // this will only worker for scheduler worker threads.
        // Returns -1 if there is an error getting the thread index
        int32_t GetWorkerThreadIndex() const;

        // returns the physical cpu count reported by the threading library
        int32_t GetCpuCount() const;

    private:
        DWORD* m_workerThreadIds = nullptr;
        HANDLE* m_workerThreadHandles = nullptr;
    };
}
