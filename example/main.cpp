#include <stdio.h>

//#include "../external/simple-stdatomic-for-VS-Clang/stdatomic.h"

//#include "fcontext/fcontext.h"

#include "kiwi/KIWI_Scheduler.h"

#include "kiwi/KIWI_Queue.h"
#include "kiwi/KIWI_Job.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

#include <atomic>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined(DEBUG) || defined(_DEBUG)
#include <crtdbg.h>
#endif // defined(DEBUG) || defined(_DEBUG)

struct TestData
{
    int num1 = 0;
    int num2 = 0;
};

void TheSuperSuperTest(KIWI_Scheduler* scheduler, void* arg)
{
    (void)scheduler;
    TestData* data = reinterpret_cast<TestData*>(arg);
    (void)data;

#if defined(DEBUG) || defined(_DEBUG)
    // Enable the D3D12 debug layer.
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
        D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
        debugController->EnableDebugLayer();
        printf("Directx 12 Test Passed\n");
    }
#endif // defined(DEBUG) || defined(_DEBUG)
}

struct PrintJobData
{
    int num = 0;
};

void TestPrintJob(KIWI_Scheduler* scheduler, void* arg)
{
    (void)scheduler;
    PrintJobData* data = reinterpret_cast<PrintJobData*>(arg);

    printf("TestPrintJob: %i\n", data->num);    
}

void TestJob(KIWI_Scheduler* scheduler, void* arg)
{
    (void)arg;
    KIWI_Job jobs[10];
    PrintJobData datas[10];

    for (int i = 0; i < 10; ++i)
    {
        datas[i].num = 9001 + i;

        jobs[i].entry = TestPrintJob;
        jobs[i].arg = &datas[i];
    }

    printf("TestJob - Start\n");
    KIWI_Counter* counter = NULL;
    KIWI_SchedulerAddJobs(scheduler, jobs, 10, KIWI_JobPriority_Normal, &counter);
    KIWI_SchedulerWaitForCounterAndFree(scheduler, counter, 0);
    printf("TestJob - End\n");
}

void StartJobs(KIWI_Scheduler* scheduler, void* arg)
{
    std::atomic_bool* endApp = reinterpret_cast<std::atomic_bool*>(arg);

    TestData testData;
    testData.num1 = 8001;
    testData.num2 = 1337;

    KIWI_Job jobs[2];
    jobs[0].entry = TheSuperSuperTest;
    jobs[0].arg = &testData;

    jobs[1].entry = TestJob;
    jobs[1].arg = NULL;

    KIWI_Counter* counter = NULL;
    KIWI_SchedulerAddJobs(scheduler, jobs, 2, KIWI_JobPriority_Normal, &counter);
    KIWI_SchedulerWaitForCounter(scheduler, counter, 0);

    KIWI_SchedulerFreeCounter(scheduler, counter);

    printf("StartJobs - End\n");

    endApp->store(true);
}

int main(int /*argc*/, char** /*argv*/)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // defined(DEBUG) || defined(_DEBUG)

    KIWI_SchedulerParams params;
    KIWI_DefaultSchedulerParams(&params);

    KIWI_Scheduler* scheduler = KIWI_CreateScheduler(&params);

    std::atomic_bool quitApp;
    quitApp.store(false);

    KIWI_Job startJob;
    startJob.entry = StartJobs;
    startJob.arg = &quitApp;
    KIWI_SchedulerAddJob(scheduler, &startJob, KIWI_JobPriority_High, NULL);

    while (!quitApp.load())
    {
        Sleep(1000);
    }

    KIWI_FreeScheduler(scheduler);

    return 0;
}
