#include "kiwi/FiberWorkerStorage.h"

using namespace kiwi;

bool FiberWorkerStorage::GetOrWaitForNextFiber(kiwi::Fiber** outFiber)
{    
    PendingJob pendingJob;
    bool haveJob = false;

    while (!m_closeWorker->load())
    {
        m_queueLock->Lock();
        if (!haveJob)
        {
            haveJob = m_queueHigh->TryGetAndPopFront(&pendingJob);
        }

        if (!haveJob)
        {
            haveJob = m_queueNormal->TryGetAndPopFront(&pendingJob);
        }

        if (!haveJob)
        {
            haveJob = m_queueLow->TryGetAndPopFront(&pendingJob);
        }
        m_queueLock->Unlock();

        if (haveJob)
        {
            *outFiber = m_fiberPool->GetFiber();

            assert(*outFiber && "No more fibers left in the fiber pool, increase the size or turn on dynamic sizing");
            
            // if we don't get a fiber just put the job back into the queue in the high priority list so it doesn't get lost
            if (*outFiber == nullptr)
            {
                m_queueLock->Lock();
                m_queueHigh->Push(pendingJob);
                m_queueLock->Unlock();
            }
            else
            {
                (*outFiber)->m_job = pendingJob.m_job;
                (*outFiber)->m_counter = pendingJob.m_counter;
                return true;
            }
        }

        if (!haveJob)
        {
            //std::unique_lock<std::mutex> lock(*m_mutex);
            // if the workers are suppose to close just return nullptr
            // the worker thread will handle shuting down
            //if (m_closeWorker->load())
            {
                //return false;
            }
            //m_conditionVariable->wait(lock);
        }
    }
    
    return false;
}
