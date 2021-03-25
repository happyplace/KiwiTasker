#include <stdio.h>

//#include "../external/simple-stdatomic-for-VS-Clang/stdatomic.h"

//#include "fcontext/fcontext.h"

#include "kiwi/KIWI_Scheduler.h"

#include "kiwi/KIWI_Queue.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined(DEBUG) || defined(_DEBUG)
#include <crtdbg.h>
#endif // defined(DEBUG) || defined(_DEBUG)

//void TheSuperSuperTest()
//{
//#if defined(DEBUG) || defined(_DEBUG)
//    // Enable the D3D12 debug layer.
//    {
//        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
//        D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
//        debugController->EnableDebugLayer();
//    }
//#endif // defined(DEBUG) || defined(_DEBUG)
//}

//void DxTest(fcontext_transfer_t t)
//{
//    printf("we do the test\n");
//    TheSuperSuperTest();
//    jump_fcontext(t.ctx, NULL);
//}

typedef struct KIWI_Job
{
    int dummy;
} KIWI_Job;

void TestJob(KIWI_Scheduler* scheduler, void* arg)
{
    (void)scheduler;
    printf("Hello There!\n");
    printf("message: %s\n", (char*)arg);
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
    KIWI_FreeScheduler(scheduler);

    KIWI_Queue* queue = KIWI_CreateQueue(sizeof(int), 5);

    bool isEmpty = KIWI_QueueIsEmpty(queue);
    (void)isEmpty;
    bool isFull = KIWI_QueueIsFull(queue);
    (void)isFull;

    int value = 1337;
    KIWI_QueuePush(queue, &value);
    
    value = 8001;
    KIWI_QueuePush(queue, &value);

    value = 9001;
    KIWI_QueuePush(queue, &value);

    value = 45;
    KIWI_QueuePush(queue, &value);

    value = 1;
    KIWI_QueuePush(queue, &value);
    
    bool isFullNow = KIWI_QueueIsFull(queue);
    (void)isFullNow;

    int outValue = 0;
    KIWI_QueuePop(queue, &outValue);

    value = 54;
    KIWI_QueuePush(queue, &value);

    while (!KIWI_QueueIsEmpty(queue))
    {
        if (KIWI_QueuePop(queue, &outValue))
        {
            printf("%i\n", outValue);
        }
    }

    if (!KIWI_QueuePop(queue, &outValue))
    {
        printf("it's empty man\n");
    }

    KIWI_FreeQueue(queue);

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
