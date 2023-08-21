/**
 * @file rad_default_allocator.h
 * @author Graham Scott
 * @brief Header file providing rad::default_allocator; a class similar
 * to std::allocator, but with additional features/optimizations.
 * @version 0.1
 * @date 2023-03-31
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 */
#ifndef RAD_DEFAULT_ALLOCATOR_H_INCLUDED
#define RAD_DEFAULT_ALLOCATOR_H_INCLUDED

#include "rad_object_utils.h"
#include "rad_memory.h"
#include <new>
#include <type_traits>
#include <cassert>

namespace rad
{
/**
 * @brief The default allocator class for libRad; used by all libRad containers by default.
 * This class allocates/rellocates/frees memory using libRad memory allocation functions.
 * 
 * @tparam T The type of data to be allocated.
 */
template<typename T>
class default_allocator
{
public:
    using value_type        = T;

    constexpr default_allocator() noexcept = default;

    template<typename U>
    constexpr default_allocator(const default_allocator<U>& other) noexcept {}

    [[nodiscard]] static inline T* allocate(std::size_t count
        RAD_IF_DEBUG_MEMORY(, debug_memory_alloc_info allocInfo))
    {
        // Allocate memory with specific alignment if necessary.
        T* ptr;
        if constexpr (alignof(T) > default_alignment)
        {
            ptr = static_cast<T*>(RAD_ALLOC_ALIGNED_DEBUG(
                sizeof(T) * count, alignof(T),
                allocInfo));
        }

        // Otherwise, just allocate default-aligned memory.
        else
        {
            ptr = static_cast<T*>(RAD_ALLOC_DEBUG(
                sizeof(T) * count, allocInfo));
        }

        // Throw if the allocation failed.
        if (!ptr)
        {
            throw std::bad_alloc();
        }

        return ptr;
    }

    [[nodiscard]] static T* reallocate(T* ptr, std::size_t oldAliveCount,
        std::size_t oldCount, std::size_t newCount
        RAD_IF_DEBUG_MEMORY(, debug_memory_alloc_info allocInfo))
    {
        // Validate arguments.
        assert((oldCount >= oldAliveCount) &&
            "oldAliveCount cannot be greater than oldCount");

        assert((ptr != nullptr || (oldAliveCount == 0 && oldCount == 0)) &&
            "The given pointer cannot be null, unless "
            "both oldAliveCount and oldCount are also 0");

        if constexpr (std::is_trivially_copyable_v<value_type>)
        {
            // NOTE: trivially-copyable types must have trivial destructors
            // and must have no non-trivial copy/move constructors/assignment
            // operators; so they are safe to perform bitwise-copies on.
            T* newMemory;
            if constexpr (alignof(T) > default_alignment)
            {
                newMemory = static_cast<T*>(RAD_REALLOC_ALIGNED_DEBUG(
                    ptr, sizeof(value_type) * newCount, alignof(T),
                    allocInfo));
            }
            else
            {
                newMemory = static_cast<T*>(RAD_REALLOC_DEBUG(
                    ptr, sizeof(value_type) * newCount, allocInfo));
            }

            // Throw if reallocation failed.
            if (!newMemory)
            {
                throw std::bad_alloc();
            }

            return newMemory;
        }
        else if (ptr != nullptr)
        {
            const auto oldAliveEnd = (ptr + oldAliveCount);

            // Enlarge memory block (actually reallocate memory block).
            if (newCount > oldCount)
            {
                // Allocate new memory block.
                const auto newMemory = allocate(newCount
                    RAD_IF_DEBUG_MEMORY(, allocInfo));

                // Move existing alive elements from old memory block to new memory block.
                uninitialized_move_strong(ptr, oldAliveEnd, newMemory);

                // NOTE: We don't construct any of the new elements;
                // they are all left uninitialized.

                // Destroy the old (now moved) alive elements.
                if constexpr (!std::is_trivially_destructible_v<value_type>)
                {
                    for (; ptr != oldAliveEnd; ++ptr)
                    {
                        rad::destruct(*ptr);
                    }
                }

                // Deallocate existing memory.
                deallocate(ptr, oldCount);

                return newMemory;
            }

            // "Shrink" memory block (leave memory block size unchanged).
            else
            {
                // Destruct any extra alive elements at the end of the existing memory block.
                const auto oldMemory = ptr;
                if constexpr (!std::is_trivially_destructible_v<value_type>)
                {
                    if (newCount < oldAliveCount)
                    {
                        for (ptr += newCount; ptr != oldAliveEnd; ++ptr)
                        {
                            rad::destruct(*ptr);
                        }
                    }
                }

                // Return the existing memory block.
                return oldMemory;
            }
        }

        // A null pointer was given; just allocate new memory.
        else
        {
            return allocate(newCount);
        }
    }

    static inline void deallocate(T* ptr, std::size_t count) noexcept
    {
        if constexpr (alignof(T) > default_alignment)
        {
            RAD_FREE_ALIGNED(ptr);
        }
        else
        {
            RAD_FREE(ptr);
        }
    }

#if RAD_USE_DEBUG_MEMORY == 1
    // If the non-debug-info versions of these functions are called,
    // just fallback to using the filePath/lineNumber of these
    // functions; it's still better than not having any debug info.

    [[nodiscard]] static inline T* allocate(std::size_t count)
    {
        return allocate(count, RAD_GET_DEBUG_MEMORY_ALLOC_INFO());
    }

    [[nodiscard]] static inline T* reallocate(T* ptr,
        std::size_t oldAliveCount, std::size_t oldCount,
        std::size_t newCount)
    {
        return reallocate(ptr, oldAliveCount, oldCount, newCount,
            RAD_GET_DEBUG_MEMORY_ALLOC_INFO());
    }
#endif
};

template<class T, class U>
constexpr bool operator==(const default_allocator<T>&, const default_allocator<U>&) noexcept
{
    return true;
}

template<class T, class U>
constexpr bool operator!=(const default_allocator<T>&, const default_allocator<U>&) noexcept
{
    return false;
}
} // rad

#endif
