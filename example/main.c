#include <stdio.h>

#include "../external/simple-stdatomic-for-VS-Clang/stdatomic.h"

#if defined(DEBUG) || defined(_DEBUG)
#include <crtdbg.h>
#endif // defined(DEBUG) || defined(_DEBUG)

int main(int argc, char** argv)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // defined(DEBUG) || defined(_DEBUG)

    atomic_llong testValue;
    atomic_store(&testValue, 0);

    long long value = atomic_load(&testValue);
    printf("Value: %llu \n", value);

    atomic_flag flag = ATOMIC_FLAG_INIT;

    while (atomic_flag_test_and_set(&flag))
        ;
    value = atomic_fetch_add(&testValue, 1);
    printf("Value: got the flag %llu \n", value);
    atomic_flag_clear(&flag);

    value = atomic_load(&testValue);
    printf("Value: %llu \n", value);

    printf("we atleast compile\n");
    return 0;
}
