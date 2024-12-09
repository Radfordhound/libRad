/// @file rad_default_allocator_impl_posix.cpp
/// @author Graham Scott
/// @brief POSIX implementation of rad_default_allocator.h
/// @date 2024-12-05
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_default_allocator.h"
#include <cstdlib>

namespace rad
{
void* default_allocator_t::allocate(
    std::size_t size,
    std::size_t alignment)
{
    const auto ptr = std::aligned_alloc(alignment, size);

    if (!ptr)
    {
        throw std::bad_alloc();
    }

    return ptr;
}

void* default_allocator_t::reallocate(
    void* ptr,
    std::size_t size,
    std::size_t alignment)
{
    // TODO: Look into ways to optimize this, such as possibly using mremap on linux?
    
    const auto newPtr = allocate(size, alignment);

    std::memcpy(newPtr, ptr, size);
    free(ptr);

    return newPtr;
}

void default_allocator_t::free(void* ptr) noexcept
{
    std::free(ptr);
}

default_allocator_t default_allocator{};
}
