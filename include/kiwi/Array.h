#include <inttypes.h>
#include <utility>

#include "kiwi/Config.h"

#ifdef KIWI_ARRAY_ERROR_CHECKING
#include <assert.h>
#endif // KIWI_ARRAY_ERROR_CHECKING

namespace kiwi
{
// Dynamic array with a fixed size container
template<typename T>
class Array
{
public:
    Array(const int32_t containerSize)
    {
#ifdef KIWI_ARRAY_ERROR_CHECKING
        assert(containerSize > 0);
        if (containerSize > 0)
#endif // KIWI_ARRAY_ERROR_CHECKING
        {
            m_containerSize = containerSize;
            m_container = new T[m_containerSize];
        }
    }

    ~Array()
    {
#ifdef KIWI_ARRAY_ERROR_CHECKING
        if (m_container != nullptr)
#endif // KIWI_ARRAY_ERROR_CHECKING
        {
            delete[] m_container;
            m_container = nullptr;
        }
    }

    void PushBack(const T& value)
    {
#ifdef KIWI_ARRAY_ERROR_CHECKING
        assert(!IsFull());
#endif // KIWI_ARRAY_ERROR_CHECKING
        m_size++;
        m_container[m_size - 1] = value;
    }

    void Remove(const int32_t index)
    {
#ifdef KIWI_ARRAY_ERROR_CHECKING
        assert(index >= 0 && index < m_size);
#endif // KIWI_ARRAY_ERROR_CHECKING

        if (m_size <= 0)
        {
            return;
        }

        if (m_size == 1)
        {
            m_size--;
            return;
        }

        m_size--;
        std::swap(m_container[index], m_container[m_size]);
    }

    T& Get(const int32_t index)
    {
#ifdef KIWI_ARRAY_ERROR_CHECKING
        assert(index >= 0 && index < m_size);
#endif // KIWI_ARRAY_ERROR_CHECKING
        return m_container[index];
    }

    T& operator[](const int32_t index)
    { 
        return Get(index);
    }

    const T& operator[](const int32_t index) const 
    { 
        return Get(index);
    }

    int32_t GetContainerSize() const { return m_containerSize; }
    int32_t GetSize() const { return m_size; }
    bool IsEmpty() const { return m_size == 0; }
    bool IsFull() const { return m_size >= m_containerSize; }

private:
    T* m_container = nullptr;
    int32_t m_containerSize = 0;
    int32_t m_size = 0;
};
}
