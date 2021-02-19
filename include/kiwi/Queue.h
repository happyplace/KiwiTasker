#pragma once

#include <assert.h>
#include <inttypes.h>

#include "kiwi/Config.h"

#ifdef KIWI_QUEUE_DYNAMIC_MODE
#include <string.h>
#endif // KIWI_QUEUE_DYNAMIC_MODE

namespace kiwi
{
// fixed sized templated queue
template<typename T>
class Queue
{
public:
    Queue(const int32_t containerSize)
    {
#if defined(KIWI_QUEUE_ERROR_CHECKING)
        assert(containerSize > 0 && "container size has to be greater than 0"); 
        if (containerSize > 0)
#endif // defined(KIWI_QUEUE_ERROR_CHECKING)
        {
            m_containerSize = containerSize;
            m_container = new T[m_containerSize];
        }
    }

    ~Queue()
    {
#if defined(KIWI_QUEUE_ERROR_CHECKING)
        if (m_container)
#endif // defined(KIWI_QUEUE_ERROR_CHECKING)
        {
            delete[] m_container;
        }
    }

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    bool IsEmpty() const
    {
        return m_headIndex == -1;
    }    

    bool IsFull() const
    {
        return m_rearIndex == m_headIndex;
    }

    void Push(T value)
    {
#if defined(KIWI_QUEUE_ERROR_CHECKING) && !defined(KIWI_QUEUE_DYNAMIC_MODE)
        assert(m_rearIndex != m_headIndex && "the queue is full");
#elif defined(KIWI_QUEUE_DYNAMIC_MODE)
        // the list is full, let's double the container size
        if (m_rearIndex == m_headIndex)
        {
            T* oldContainer = m_container;
            int32_t oldContainerSize = m_containerSize;

            m_containerSize = static_cast<int32_t>(m_containerSize * KIWI_QUEUE_DYNAMIC_INCREASE_MULTIPLIER);
            m_container = new T[m_containerSize];

            memcpy(m_container, oldContainer, oldContainerSize);

            delete[] oldContainer;
        }
#endif // defined(KIWI_QUEUE_ERROR_CHECKING) && !defined(KIWI_QUEUE_DYNAMIC_MODE)

        m_container[m_rearIndex] = value;

        // move head index if this is the list was previous empty
        m_headIndex = m_headIndex < 0 ? m_rearIndex : m_headIndex;

        // move rear index by one, and wrap around to the front of the array if we're at the end
        m_rearIndex = (++m_rearIndex) >= m_containerSize ? 0 : m_rearIndex;
    }

    bool TryGetAndPopFront(T* outValue)
    {
        bool success = false;

        if (m_headIndex >= 0)
        {
            *outValue = m_container[m_headIndex];
            success = true;
        }

        // move head index by one, and wrap around to the front of the array if we're at the end
        m_headIndex = (++m_headIndex) >= m_containerSize ? 0 : m_headIndex;

        return success;
    }

private:
    int32_t m_containerSize = 0;
    T* m_container = nullptr;

    // points to the current head index or -1 if the list is empty
    int32_t m_headIndex = -1;

    // points to the next available index
    int32_t m_rearIndex = 0;    
};
}
