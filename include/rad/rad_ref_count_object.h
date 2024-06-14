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
    std::atomic_size_t refCount_;
    
public:
    /// @brief Atomically adds a reference to the reference counter.
    ///
    /// This operation is entirely atomic, and thus can be used safely
    /// across multiple threads without any additional synchronization.
    void add_ref() noexcept
    {
        refCount_.fetch_add(1, std::memory_order_relaxed);
    }

    /// @brief Atomically subtracts a reference from the reference counter.
    ///
    /// This operation is entirely atomic, and thus can be used safely
    /// across multiple threads without any additional synchronization.
    ///
    /// @return Returns false if this was the last reference (i.e. the
    /// reference count is now 0), and true otherwise.
    bool release_ref() noexcept
    {
        const auto prevRefCount = refCount_.fetch_sub(1, std::memory_order_acq_rel);

        assert(prevRefCount != 0 &&
            "release_ref() was called on a ref_count_object whose "
            "reference count was 0"
        );

        return (prevRefCount != 1);
    }

    /// @brief Constructs a new ref_count_object.
    /// @param startRefCountAtZero True if the reference counter should
    /// start at 0; false if it should start at 1.
    constexpr ref_count_object(bool startRefCountAtZero) noexcept
        : refCount_(startRefCountAtZero ? 0 : 1)
    {
    }
};
}

#endif
