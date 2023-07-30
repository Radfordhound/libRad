/**
 * @file rad_allocator_traits.h
 * @author Graham Scott
 * @brief Helper class for working with allocators, similar to std::allocator_traits.
 * @version 0.1
 * @date 2023-04-03
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 */
#ifndef RAD_ALLOCATOR_TRAITS_H_INCLUDED
#define RAD_ALLOCATOR_TRAITS_H_INCLUDED

#include "rad_object_utils.h"
#include "rad_memory.h"
#include <new>
#include <memory>
#include <utility>
#include <type_traits>
#include <cassert>

namespace rad
{
namespace detail_
{
#if RAD_USE_DEBUG_MEMORY == 1
    template<class AllocatorTraits, class = void>
    struct allocator_has_debug_allocate : std::false_type {};

    template<class AllocatorTraits>
    struct allocator_has_debug_allocate<AllocatorTraits, std::void_t<decltype(
        std::declval<typename AllocatorTraits::allocator_type&>().allocate(
            std::declval<const typename AllocatorTraits::size_type&>(), // count
            std::declval<const debug_memory_alloc_info&>()              // allocInfo
        ))>> : std::true_type {};

    template<class AllocatorTraits, class = void>
    struct allocator_has_debug_reallocate : std::false_type {};

    template<class AllocatorTraits>
    struct allocator_has_debug_reallocate<AllocatorTraits, std::void_t<decltype(
        std::declval<typename AllocatorTraits::allocator_type&>().reallocate(
            std::declval<const typename AllocatorTraits::pointer&>(),   // ptr
            std::declval<const typename AllocatorTraits::size_type&>(), // oldAliveCount
            std::declval<const typename AllocatorTraits::size_type&>(), // oldCount
            std::declval<const typename AllocatorTraits::size_type&>(), // newCount
            std::declval<const debug_memory_alloc_info&>()              // allocInfo
        ))>> : std::true_type {};
#else
    template<class AllocatorTraits>
    struct allocator_has_debug_allocate : std::false_type {};

    template<class AllocatorTraits>
    struct allocator_has_debug_reallocate : std::false_type {};
#endif

template<class AllocatorTraits, class = void>
struct allocator_has_reallocate : std::false_type {};

template<class AllocatorTraits>
struct allocator_has_reallocate<AllocatorTraits, std::void_t<decltype(
    std::declval<typename AllocatorTraits::allocator_type&>().reallocate(
        std::declval<const typename AllocatorTraits::pointer&>(),   // ptr
        std::declval<const typename AllocatorTraits::size_type&>(), // oldAliveCount
        std::declval<const typename AllocatorTraits::size_type&>(), // oldCount
        std::declval<const typename AllocatorTraits::size_type&>()  // newCount
    ))>> : std::true_type {};

template<class AllocatorTraits, class = void>
struct allocator_has_destroy : std::false_type {};

template<class AllocatorTraits>
struct allocator_has_destroy<AllocatorTraits, std::void_t<decltype(
    std::declval<typename AllocatorTraits::allocator_type&>().destroy(
        std::declval<const typename AllocatorTraits::pointer&>()    // p
    ))>> : std::true_type {};
} // detail_

template<class Allocator>
struct allocator_traits : public std::allocator_traits<Allocator>
{
private:
    using this_type_    = allocator_traits<Allocator>;

public:
    static constexpr bool has_debug_allocate =
        detail_::allocator_has_debug_allocate<this_type_>::value;

    static constexpr bool has_reallocate =
        detail_::allocator_has_reallocate<this_type_>::value;

    static constexpr bool has_debug_reallocate =
        detail_::allocator_has_debug_reallocate<this_type_>::value;

    static constexpr bool has_destroy =
        detail_::allocator_has_destroy<this_type_>::value;

    [[nodiscard]] static inline typename allocator_traits::pointer allocate(
        Allocator& allocator, typename allocator_traits::size_type count
        RAD_IF_DEBUG_MEMORY(, debug_memory_alloc_info allocInfo))
    {
#if RAD_USE_DEBUG_MEMORY == 1
        if constexpr (has_debug_allocate)
        {
            return allocator.allocate(count, allocInfo);
        }
        else
#endif
        {
            return allocator.allocate(count);
        }
    }

    [[nodiscard]] static typename allocator_traits::pointer reallocate(
        Allocator& allocator, typename allocator_traits::pointer ptr,
        typename allocator_traits::size_type oldAliveCount,
        typename allocator_traits::size_type oldCount,
        typename allocator_traits::size_type newCount
        RAD_IF_DEBUG_MEMORY(, debug_memory_alloc_info allocInfo))
    {
        // Validate arguments.
        assert((oldCount >= oldAliveCount) &&
            "oldAliveCount cannot be greater than oldCount");

        assert((ptr != nullptr || (oldAliveCount == 0 && oldCount == 0)) &&
            "The given pointer cannot be null, unless "
            "both oldAliveCount and oldCount are also 0");

        // Call reallocate on the allocator, if said function exists.
#if RAD_USE_DEBUG_MEMORY == 1
        if constexpr (has_debug_reallocate)
        {
            return allocator.reallocate(ptr, oldAliveCount,
                oldCount, newCount, allocInfo);
        }
        else
#endif
        if constexpr (has_reallocate)
        {
            return allocator.reallocate(
                ptr, oldAliveCount, oldCount, newCount);
        }

        // Otherwise, fallback to a "default" reallocate implementation.
        else if constexpr (std::is_trivially_copyable_v<
            typename allocator_traits::value_type>)
        {
            // NOTE: trivially-copyable types must have trivial destructors
            // and must have no non-trivial copy/move constructors/assignment
            // operators; so they are safe to perform bitwise-copies on.
            typename allocator_traits::pointer newMemory;
            if constexpr (alignof(typename allocator_traits::value_type) > default_alignment)
            {
                newMemory = static_cast<typename allocator_traits::pointer>(
                    RAD_REALLOC_ALIGNED_DEBUG(ptr,
                        sizeof(typename allocator_traits::value_type) * newCount,
                        alignof(typename allocator_traits::value_type), allocInfo));
            }
            else
            {
                newMemory = static_cast<typename allocator_traits::pointer>(
                    RAD_REALLOC_DEBUG(ptr,
                        sizeof(typename allocator_traits::value_type) * newCount,
                        allocInfo));
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
                const auto newMemory = allocate(allocator, newCount);

                // Move existing alive elements from old memory block to new memory block.
                uninitialized_move_strong(ptr, oldAliveEnd, newMemory);

                // NOTE: We don't construct any of the new elements;
                // they are all left uninitialized.

                // Destroy the old (now moved) alive elements.
                if constexpr (!std::is_trivially_destructible_v<
                    typename allocator_traits::value_type>)
                {
                    for (; ptr != oldAliveEnd; ++ptr)
                    {
                        destroy(allocator, ptr);
                    }
                }

                // Deallocate existing memory.
                deallocate(allocator, ptr, oldCount);

                return newMemory;
            }

            // "Shrink" memory block (leave memory block size unchanged).
            else
            {
                // Destruct any extra alive elements at the end of the existing memory block.
                const auto oldMemory = ptr;
                if constexpr (!std::is_trivially_destructible_v<
                    typename allocator_traits::value_type>)
                {
                    if (newCount < oldAliveCount)
                    {
                        for (ptr += newCount; ptr != oldAliveEnd; ++ptr)
                        {
                            destroy(allocator, ptr);
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
            return allocate(allocator, newCount);
        }
    }

#if RAD_USE_DEBUG_MEMORY == 1
    // If the non-debug-info versions of these functions are called,
    // just fallback to using the filePath/lineNumber of these
    // functions; it's still better than not having any debug info.

    [[nodiscard]] static inline typename allocator_traits::pointer allocate(
        Allocator& allocator, typename allocator_traits::size_type count)
    {
        return allocate(allocator, count, RAD_GET_DEBUG_MEMORY_ALLOC_INFO());
    }

    [[nodiscard]] static inline typename allocator_traits::pointer reallocate(
        Allocator& allocator, typename allocator_traits::pointer ptr,
        typename allocator_traits::size_type oldAliveCount,
        typename allocator_traits::size_type oldCount,
        typename allocator_traits::size_type newCount)
    {
        return reallocate(allocator, ptr, oldAliveCount,
            oldCount, newCount, RAD_GET_DEBUG_MEMORY_ALLOC_INFO());
    }
#endif
};
} // rad

#endif
