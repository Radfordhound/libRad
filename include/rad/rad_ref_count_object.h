/// @file rad_ref_count_object.h
/// @author Graham Scott
/// @brief Header file providing rad::ref_count_object; a base class
/// which can be inherited from to provide atomic thread-safe reference
/// counting.
/// @date 2024-06-13
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_REF_COUNT_OBJECT_H_INCLUDED
#define RAD_REF_COUNT_OBJECT_H_INCLUDED

#include <atomic>
#include <cassert>

namespace rad
{
class ref_count_object
{
    mutable std::atomic_size_t refCount_;
    
public:
    /// @brief Atomically adds a reference to the reference counter.
    ///
    /// This operation is entirely atomic, and thus can be used safely
    /// across multiple threads without any additional synchronization.
    void add_ref() const noexcept
    {
        refCount_.fetch_add(1, std::memory_order_relaxed);
    }

    /// @brief Atomically subtracts a reference from the reference counter.
    ///
    /// This operation is entirely atomic, and thus can be used safely
    /// across multiple threads without any additional synchronization.
    ///
    /// @return Returns true if this was the last reference (i.e. the
    /// reference count is now 0), and false otherwise.
    bool release_ref() const noexcept
    {
        const auto prevRefCount = refCount_.fetch_sub(1, std::memory_order_acq_rel);

        assert(prevRefCount != 0 &&
            "release_ref() was called on a ref_count_object whose "
            "reference count was 0"
        );

        return (prevRefCount == 1);
    }

    /// @brief Constructs a new ref_count_object, with the
    /// reference counter set to 0.
    constexpr ref_count_object() noexcept
        : refCount_(0)
    {
    }

    /// @brief Constructs a new ref_count_object, with the
    /// reference counter set to initialRefCount.
    /// @param initialRefCount What value should the reference counter start at.
    constexpr ref_count_object(std::size_t initialRefCount) noexcept
        : refCount_(initialRefCount)
    {
    }
};
}

#endif
