#include "kiwi/SchedulerThreadImplWinThreads.h"

#include <assert.h>

#ifndef WIN_ASSERT
#define WIN_ASSERT(x) \
{ \
    auto result__ = (x); \
    if (result__ == 0) \
    { \
        LPVOID lpMsgBuf; \
        DWORD lastError = GetLastError(); \
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, \
            NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL); \
        MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Kiwi Thread Error"), MB_OK); \
        LocalFree(lpMsgBuf); \
    } \
}
#endif // WIN_ASSERT

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

constexpr DWORD MS_VC_EXCEPTION = 0x406D1388;
constexpr int32_t MS_CORES_PER_GROUP = 64;

using namespace kiwi;

SchedulerThreadImpl::SchedulerThreadImpl()
{
    const int32_t cpuCount = GetCpuCount();

    m_workerThreadIds = new DWORD[cpuCount];
    m_workerThreadHandles = new HANDLE[cpuCount];
}

SchedulerThreadImpl::~SchedulerThreadImpl()
{
    const int32_t cpuCount = GetCpuCount();
    for (int32_t i = 0; i < cpuCount; ++i)
    {
        CloseHandle(m_workerThreadHandles[i]);
    }

    delete[] m_workerThreadIds;
    delete[] m_workerThreadHandles;
}

void SchedulerThreadImpl::JoinWorkerThreads()
{
    const int32_t cpuCount = GetCpuCount();

    WaitForMultipleObjects(cpuCount, m_workerThreadHandles, TRUE, INFINITE);
}

void SchedulerThreadImpl::CreateThread(const char* threadName, int32_t threadAffinity, DWORD(*threadFunction) (LPVOID), void* threadFunctionArg)
{
    m_workerThreadHandles[threadAffinity] = ::CreateThread(NULL, 0, threadFunction, threadFunctionArg, 0, &m_workerThreadIds[threadAffinity]);
    
    assert(m_workerThreadHandles[threadAffinity] != NULL && "Failed to create worker thread");

    // when your system has more than 64 cores they're seperated into groups, this determines the group then assigns the thread to a specific
    // core within that group
    GROUP_AFFINITY groupAffinity;
    ZeroMemory(&groupAffinity, sizeof(GROUP_AFFINITY));
    groupAffinity.Mask = static_cast<DWORD_PTR>(1) << static_cast<int32_t>(threadAffinity % MS_CORES_PER_GROUP);
    groupAffinity.Group = static_cast<WORD>(threadAffinity / MS_CORES_PER_GROUP);    
    WIN_ASSERT(SetThreadGroupAffinity(m_workerThreadHandles[threadAffinity], &groupAffinity, NULL));

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = m_workerThreadIds[threadAffinity];
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

int32_t SchedulerThreadImpl::GetWorkerThreadIndex() const
{
    GROUP_AFFINITY groupAffinity;
    ZeroMemory(&groupAffinity, sizeof(GROUP_AFFINITY));
    WIN_ASSERT(GetThreadGroupAffinity(GetCurrentThread(), &groupAffinity));

    const int32_t threadIndex = static_cast<int32_t>(groupAffinity.Group * MS_CORES_PER_GROUP);
    for (int32_t i = 0; i < MS_CORES_PER_GROUP; ++i)
    {        
        const DWORD_PTR mask = static_cast<DWORD_PTR>(1) << i;
        // we assume this will only be called by worker threads so when we find the first set affinity
        // we add it to the threadIndex and return that value
        if ((groupAffinity.Mask & mask) != 0)
        {
            return threadIndex + i;
        }
    }

    assert(!"Why is no cpu set, is this being called outside of a scheduler thread?");
    return -1;
}

int32_t SchedulerThreadImpl::GetCpuCount() const
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return static_cast<int32_t>(sysinfo.dwNumberOfProcessors);
}

void SchedulerThreadImpl::BlockSignalsOnWorkerThread() const
{
    // from my research this isn't something that happens on windows so we do nothing here
}
