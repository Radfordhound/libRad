/// @file rad_default_allocator.h
/// @author Graham Scott
/// @brief Header file providing rad::default_allocator; a class similar
/// to std::allocator, but with additional features/optimizations.
/// @date 2023-03-31
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_DEFAULT_ALLOCATOR_H_INCLUDED
#define RAD_DEFAULT_ALLOCATOR_H_INCLUDED

#include "rad_base.h"
#include "rad_allocator.h"

namespace rad
{
class default_allocator_t
    : public allocator
{
public:
    RAD_API void* allocate(
        std::size_t size,
        std::size_t alignment
    ) override;

    RAD_API void* reallocate(
        void* ptr,
        std::size_t size,
        std::size_t alignment
    ) override;

    RAD_API void free(void* ptr) noexcept override;
};

RAD_API [[maybe_unused]] extern default_allocator_t default_allocator;
}

#endif
