#include <stdio.h>

//#include "../external/simple-stdatomic-for-VS-Clang/stdatomic.h"

#include "fcontext/fcontext.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

#if defined(DEBUG) || defined(_DEBUG)
#include <crtdbg.h>
#endif // defined(DEBUG) || defined(_DEBUG)

void TheSuperSuperTest()
{
#if defined(DEBUG) || defined(_DEBUG)
    // Enable the D3D12 debug layer.
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
        D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
        debugController->EnableDebugLayer();
    }
#endif // defined(DEBUG) || defined(_DEBUG)
}

void DxTest(fcontext_transfer_t t)
{
    printf("we do the test\n");
    TheSuperSuperTest();
    jump_fcontext(t.ctx, NULL);
}

int main(int /*argc*/, char** /*argv*/)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // defined(DEBUG) || defined(_DEBUG)

    fcontext_stack_t s = create_fcontext_stack(16 * 1024);

    fcontext_t ctx = make_fcontext(s.sptr, s.ssize, DxTest);
    jump_fcontext(ctx, NULL);
    printf("we got back\n");

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
