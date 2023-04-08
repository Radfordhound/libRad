/**
 * @file rad_stack_or_heap_memory.h
 * @author Graham Scott
 * @brief Header file providing rad::stack_or_heap_memory.
 * @version 0.1
 * @date 2023-04-07
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 */
#ifndef RAD_STACK_OR_HEAP_MEMORY_H_INCLUDED
#define RAD_STACK_OR_HEAP_MEMORY_H_INCLUDED

#include "rad_memory.h"
#include <stdexcept>
#include <utility>
#include <cstring>

namespace rad
{
template<std::size_t Size, std::size_t Alignment = alignof(unsigned char)>
class stack_or_heap_memory
{
    void* data_;
    unsigned char alignas(Alignment) stackMemory_[Size];

    void allocate_(std::size_t size)
    {
        // Stack "allocation".
        if (size <= Size)
        {
            data_ = stackMemory_;
        }

        // Heap allocation.
        else
        {
            if constexpr (Alignment > default_alignment)
            {
                data_ = RAD_ALLOC_ALIGNED(size, Alignment);
            }
            else
            {
                data_ = RAD_ALLOC(size);
            }

            if (!data_)
            {
                throw std::bad_alloc();
            }
        }
    }

    void take_ownership_(stack_or_heap_memory&& other) noexcept
    {
        // Just copy heap memory pointer.
        if (other.is_heap())
        {
            data_ = other.data_;
            other.data_ = other.stackMemory_;
        }

        // Copy stack memory.
        else
        {
            std::memcpy(stackMemory_, other.stackMemory_, Size);
            data_ = stackMemory_;
        }
    }

    void deallocate_() noexcept
    {
        // Free any heap allocations.
        if (is_heap())
        {
            if constexpr (Alignment > default_alignment)
            {
                RAD_FREE_ALIGNED(data_);
            }
            else
            {
                RAD_FREE(data_);
            }
        }
    }

public:
    template<typename T = void>
    inline const T* data() const noexcept
    {
        return static_cast<const T*>(data_);
    }

    template<typename T = void>
    inline T* data() noexcept
    {
        return static_cast<T*>(data_);
    }

    inline bool is_heap() const noexcept
    {
        return (data_ != stackMemory_);
    }

    void reallocate(std::size_t size)
    {
        if (is_heap())
        {
            // Attempt to reallocate heap memory.

            // NOTE: We always just reallocate in this case, even if we're reallocating
            // to a size which is small enough to fit into the stack memory, since
            // it's faster to just "truncate" the existing heap memory block (effectively
            // we're doing nothing) than it is to check the size, memcpy the heap memory
            // into the stack memory, and free the heap memory.

            void* newData;
            if constexpr (Alignment > default_alignment)
            {
                newData = RAD_REALLOC_ALIGNED(data_, size, Alignment);
            }
            else
            {
                newData = RAD_REALLOC(data_, size);
            }

            // Throw if reallocation failed.
            if (!newData)
            {
                throw std::bad_alloc();
            }

            // Assign new pointer if reallocation succeeded.

            // NOTE: We do this only if reallocation succeeded to
            // provide a strong exception guarantee.

            data_ = newData;
        }
        else if (size > Size)
        {
            // Attempt to allocate heap memory.
            void* newData;
            if constexpr (Alignment > default_alignment)
            {
                newData = RAD_ALLOC_ALIGNED(size, Alignment);
            }
            else
            {
                newData = RAD_ALLOC(size);
            }

            // Throw if allocation failed.
            if (!newData)
            {
                throw std::bad_alloc();
            }

            // Copy existing stack memory into new heap memory.

            // NOTE: Since size > Size, the additional memory that comes
            // after Size will be left uninitialized, similar to realloc.

            std::memcpy(newData, data_, Size);

            // Assign new pointer if allocation succeeded.

            // NOTE: We do this only if allocation succeeded to
            // provide a strong exception guarantee.

            data_ = newData;
        }

        // NOTE: In the event that size <= Size, we don't have to do anything.
    }

    void deallocate() noexcept
    {
        deallocate_();
        data_ = stackMemory_;
    }

    stack_or_heap_memory& operator=(const stack_or_heap_memory& other) = delete;

    inline stack_or_heap_memory& operator=(stack_or_heap_memory&& other) noexcept
    {
        if (&other != this)
        {
            deallocate_();
            take_ownership_(std::move(other));
        }

        return *this;
    }

    inline stack_or_heap_memory() noexcept :
        data_(stackMemory_) {}

    inline stack_or_heap_memory(std::size_t size)
    {
        allocate_(size);
    }

    stack_or_heap_memory(const stack_or_heap_memory& other) = delete;

    inline stack_or_heap_memory(stack_or_heap_memory&& other) noexcept
    {
        take_ownership_(std::move(other));
    }

    inline ~stack_or_heap_memory()
    {
        deallocate_();
    }
};
} // rad

#endif
