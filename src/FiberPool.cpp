#include "kiwi/FiberPool.h"

#include <assert.h>

using namespace kiwi;

FiberPool::FiberPool(int32_t poolSize)
{
#ifdef KIWI_FIBERPOOL_ERROR_CHECKING
    assert(poolSize > 0 && "pool size has to be greater than 0"); 
    if (poolSize > 0)
#endif // KIWI_FIBERPOOL_ERROR_CHECKING
    {        
#ifdef KIWI_FIBERPOOL_DYNAMIC_MODE
        // add the first pool to the dynamic pool link
        FiberPoolLinks* poolLink = new FiberPoolLinks();
        poolLink->m_buffer = CreatePool(poolSize);
        poolLink->m_next = nullptr;

        m_firstPoolLink = poolLink;

        m_firstFree = &poolLink->m_buffer[0];

        // keep pool size so we know how big to make additional buffers
        m_poolSize = poolSize;
#else
        m_pool = CreatePool(poolSize);

        m_firstFree = &m_pool[0];
#endif // KIWI_FIBERPOOL_DYNAMIC_MODE
    }
}

FiberPool::~FiberPool()
{
#ifdef KIWI_FIBERPOOL_DYNAMIC_MODE
    FiberPoolLinks* poolLink = m_firstPoolLink;
    while (poolLink != nullptr)
    {
        FiberPoolLinks* link = poolLink;
        delete[] poolLink->m_buffer;
        poolLink = poolLink->m_next;
        delete link;
    }
#else
#ifdef KIWI_FIBERPOOL_ERROR_CHECKING
    if (m_pool)
#endif // KIWI_FIBERPOOL_ERROR_CHECKING
    {
        delete[] m_pool;
    }
#endif // KIWI_FIBERPOOL_DYNAMIC_MODE
}

Fiber* FiberPool::CreatePool(int32_t poolSize) const
{
    Fiber* pool = new Fiber[poolSize];

    // make every fiber in the pool point to the next fiber in the list
    for (int32_t i = 0; i < poolSize - 1; ++i)
    {
        pool[i].next_ = &pool[i + 1];
    }

    // make the last fiber point to nullptr
    pool[poolSize - 1].next_ = nullptr;

    return pool;
}

Fiber* FiberPool::GetFiber()
{
    Fiber* fiber = nullptr;

    m_lock.Lock();  

    if (m_firstFree != nullptr)
    {
        fiber = m_firstFree;
        m_firstFree = fiber->next_;
    }
#ifdef KIWI_FIBERPOOL_DYNAMIC_MODE
    else
    {
        // pool is empty create another buffer
        FiberPoolLinks* poolLink = new FiberPoolLinks();
        poolLink->m_buffer = CreatePool(m_poolSize);
        poolLink->m_next = nullptr;
        
        // return the first fiber in the buffer to the caller since we just made more for them
        fiber = &poolLink->m_buffer[0];

        // point at the second element before we're giving the first element to the caller
        m_firstFree = &poolLink->m_buffer[1];

        // append to end of fiber pool links list
        FiberPoolLinks* link = m_firstPoolLink;
        while (link->m_next != nullptr)
        {
            link = link->m_next;
        }
        link->m_next = poolLink;
    }
#endif // KIWI_FIBERPOOL_DYNAMIC_MODE

    m_lock.Unlock();

#ifdef KIWI_FIBERPOOL_ERROR_CHECKING
    assert(fiber && "no more free fibers in pool increase pool size");
#endif // KIWI_FIBERPOOL_ERROR_CHECKING

    return fiber;
}

void FiberPool::ReturnFiber(Fiber* fiber)
{
#ifdef KIWI_FIBERPOOL_ERROR_CHECKING
    assert(fiber && "trying to release a null fiber");
#endif // KIWI_FIBERPOOL_ERROR_CHECKING

    m_lock.Lock();
    fiber->next_ = m_firstFree;
    m_firstFree = fiber;
    m_lock.Unlock();
}
