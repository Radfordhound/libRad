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
// TODO: Add allocator.
template<typename T, std::size_t MaxStackCount>
class stack_or_heap_array
{
    using buffer_type_ = stack_or_heap_memory<
        sizeof(T) * MaxStackCount,
        alignof(T)>;

    std::size_t     count_ = 0;
    buffer_type_    buffer_;

    void clear_() noexcept
    {
        // Destruct any existing elements and free any existing heap memory.
        destruct(begin(), end());
        count_ = 0;
        buffer_.deallocate_(); // NOTE: This does NOT reset the buffer_.data_ pointer!
    }

public:
    inline std::size_t size() const noexcept
    {
        return count_;
    }

    inline const T* begin() const noexcept
    {
        return buffer_.data<T>();
    }

    inline T* begin() noexcept
    {
        return buffer_.data<T>();
    }

    inline const T* end() const noexcept
    {
        return (buffer_.data<T>() + count_);
    }

    inline T* end() noexcept
    {
        return (buffer_.data<T>() + count_);
    }

    inline void clear() noexcept
    {
        clear_();
        buffer_.data_ = buffer_.stackMemory_;
    }

    template<typename... Args>
    void assign(std::size_t count, const Args&... args)
    {
        // Destruct any existing elements.
        destruct(begin(), end());
        count_ = 0;

        // Reallocate memory block, not preserving existing data.
        buffer_.reallocate(sizeof(T) * count, false);

        // Direct-construct new elements, and set new count.
        uninitialized_direct_construct(begin(), begin() + count, args...);
        count_ = count;
    }

    // TODO: Add other overloads of assign.

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
            // Reset the array without setting a new buffer_.data_ pointer.
            clear_();

            // If the other array is using heap memory, just take ownership of its data pointer.
            if (other.buffer_.is_heap())
            {
                buffer_.data_ = other.buffer_.data_;
                other.buffer_.data_ = other.buffer_.stackMemory_;
            }

            // If the other array is using stack memory, move its elements into our stack memory
            // and destruct the now-empty elements in the existing array.
            else
            {
                buffer_.data_ = buffer_.stackMemory_;
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
        count_(count),
        buffer_(sizeof(T) * count)
    {
        uninitialized_direct_construct(begin(), end(), args...);
    }

    // TODO: Add iterator constructor.

    stack_or_heap_array(const stack_or_heap_array& other) :
        count_(other.count_),
        buffer_(sizeof(T) * other.count_)
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
        if (other.buffer_.is_heap())
        {
            buffer_.data_ = other.buffer_.data_;
            other.buffer_.data_ = other.buffer_.stackMemory_;
        }

        // If the other array is using stack memory, move its elements into our stack memory
        // and destruct the now-empty elements in the existing array.
        else
        {
            // NOTE: buffer_.data_ is already set to buffer_.stackMemory_ here thanks to
            // stack_or_heap_memory's default constructor.
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
