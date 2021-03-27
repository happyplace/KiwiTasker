#include <stdio.h>

//#include "../external/simple-stdatomic-for-VS-Clang/stdatomic.h"

//#include "fcontext/fcontext.h"

#include "kiwi/KIWI_Scheduler.h"

#include "kiwi/KIWI_Queue.h"
#include "kiwi/KIWI_Job.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

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
    KIWI_SchedulerAddJobs(scheduler, jobs, 10, KIWI_JobPriority_Normal);
    printf("TestJob - End\n");
}

//int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, PSTR /*lpCmdLine*/, int /*nCmdShow*/)
int main(int /*argc*/, char** /*argv*/)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // defined(DEBUG) || defined(_DEBUG)

    KIWI_SchedulerParams params;
    KIWI_DefaultSchedulerParams(&params);

    KIWI_Scheduler* scheduler = KIWI_CreateScheduler(&params);

    TestData testData;
    testData.num1 = 8001;
    testData.num2 = 1337;

    KIWI_Job job;
    job.entry = TheSuperSuperTest;
    job.arg = &testData;

    KIWI_SchedulerAddJob(scheduler, &job, KIWI_JobPriority_Normal);

    job.entry = TestJob;
    job.arg = NULL;

    KIWI_SchedulerAddJob(scheduler, &job, KIWI_JobPriority_Normal);

    Sleep(1500);

    KIWI_FreeScheduler(scheduler);

    //fcontext_stack_t s = create_fcontext_stack(16 * 1024);

    //fcontext_t ctx = make_fcontext(s.sptr, s.ssize, DxTest);
    //jump_fcontext(ctx, NULL);
    //printf("we got back\n");

    //atomic_llong testValue;
    //atomic_store(&testValue, 0);

    //long long value = atomic_load(&testValue);
    //printf("Value: %llu \n", value);

    //atomic_flag flag = ATOMIC_FLAG_INIT;

    //while (atomic_flag_test_and_set(&flag))
    //    ;
    //value = atomic_fetch_add(&testValue, 1);
    //printf("Value: got the flag %llu \n", value);
    //atomic_flag_clear(&flag);

    //value = atomic_load(&testValue);
    //printf("Value: %llu \n", value);

    //printf("we atleast compile\n");
    return 0;
}
