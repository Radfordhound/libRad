/**
 * @file rad_stack_or_heap_array.h
 * @author Graham Scott
 * @brief Header file providing rad::stack_or_heap_array.
 * @version 0.1
 * @date 2023-06-05
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 */
#ifndef RAD_STACK_OR_HEAP_ARRAY_H_INCLUDED
#define RAD_STACK_OR_HEAP_ARRAY_H_INCLUDED

#include "rad_stack_or_heap_memory.h"
#include "rad_object_utils.h"
#include <utility>

namespace rad
{
template<typename T, std::size_t MaxStackCount>
class stack_or_heap_array
{
    using buffer_type_ = stack_or_heap_memory<
        sizeof(T) * MaxStackCount,
        alignof(T)>;

    buffer_type_    data_;
    std::size_t     count_ = 0;

public:
    inline std::size_t size() const noexcept
    {
        return count_;
    }

    inline const T* begin() const noexcept
    {
        return data_.data<T>();
    }

    inline T* begin() noexcept
    {
        return data_.data<T>();
    }

    inline const T* end() const noexcept
    {
        return (data_.data<T>() + count_);
    }

    inline T* end() noexcept
    {
        return (data_.data<T>() + count_);
    }

    inline const T& operator[](std::size_t index) const
    {
        return *(begin() + index);
    }

    inline T& operator[](std::size_t index)
    {
        return *(begin() + index);
    }

    // TODO: copy operator=

    stack_or_heap_array& operator=(stack_or_heap_array&& other)
        noexcept(noexcept(uninitialized_move_strong(
            std::declval<T*>(), std::declval<T*>(),
            std::declval<T*>())))
    {
        if (&other != this)
        {
            // Destruct any existing elements and free any existing heap memory.
            destruct(begin(), end());
            count_ = 0;
            data_.deallocate_();

            // If the other array is using heap memory, just take ownership of its data pointer.
            if (other.data_.is_heap())
            {
                data_.data = other.data_;
                other.data_.data_ = other.data_.stackMemory_;
            }

            // If the other array is using stack memory, move its elements into our stack memory
            // and destruct the now-empty elements in the existing array.
            else
            {
                uninitialized_move_strong(other.begin(), other.end(), begin());
                destruct(other.begin(), other.end());
            }

            count_ = other.count_;
            other.count_ = 0;
        }

        return *this;
    }

    stack_or_heap_array() noexcept = default;

    template<typename... Args>
    stack_or_heap_array(std::size_t count, const Args&... args) :
        data_(sizeof(T) * count),
        count_(count)
    {
        uninitialized_direct_construct(begin(), end(), args...);
    }

    stack_or_heap_array(const stack_or_heap_array& other) :
        data_(sizeof(T) * other.count_),
        count_(other.count_)
    {
        std::uninitialized_copy(other.begin(), other.end(), begin());
    }

    stack_or_heap_array(stack_or_heap_array&& other)
        noexcept(noexcept(uninitialized_move_strong(
            std::declval<T*>(), std::declval<T*>(),
            std::declval<T*>()))) :

        count_(other.count_)
    {
        // If the other array is using heap memory, just take ownership of its data pointer.
        if (other.data_.is_heap())
        {
            data_.data_ = other.data_.data_;
            other.data_.data_ = other.data_.stackMemory_;
        }

        // If the other array is using stack memory, move its elements into our stack memory
        // and destruct the now-empty elements in the existing array.
        else
        {
            uninitialized_move_strong(other.begin(), other.end(), begin());
            destruct(other.begin(), other.end());
        }

        other.count_ = 0;
    }

    ~stack_or_heap_array()
    {
        destruct(begin(), end());
    }
};
} // rad

#endif
