#include "kiwi/KIWI_ThreadImplPthread.h"

#include <malloc.h>

typedef struct KIWI_ThreadImpl
{
    int dummy;
} KIWI_ThreadImpl;

struct KIWI_ThreadImpl* KIWI_ThreadImplCreateAndStartWorkerThreads(int workerCount, void*(*threadFunction) (void*))
{
    return NULL;
}

void KIWI_ThreadImplShutdownWorkerThreads(struct KIWI_ThreadImpl* threadImpl)
{

}

void KIWI_ThreadImplBlockSignalsOnWorkerThread()
{

}

int KIWI_ThreadImplGetCpuCount()
{
    return -1;
}

void KIWI_ThreadImplNotifyOneWorkerThread(struct KIWI_ThreadImpl* threadImpl)
{

}

void KIWI_ThreadImplNotifyAllWorkerThreads(struct KIWI_ThreadImpl* threadImpl)
{

}

void KIWI_ThreadImplSleepUntilJobAdded(struct KIWI_ThreadImpl* threadImpl, atomic_bool* quitWorkerThreads)
{

}

void KIWI_ThreadImplSignalWorkerThreadsToQuit(struct KIWI_ThreadImpl* threadImpl, atomic_bool* quitWorkerThreads)
{

}

int KIWI_ThreadImplGetWorkerThreadIndex()
{
    return -1;
}
