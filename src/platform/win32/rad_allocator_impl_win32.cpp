/// @file rad_allocator_impl_win32.cpp
/// @author Graham Scott
/// @brief Windows implementation of rad_allocator.h
/// @date 2024-12-05
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_allocator.h"
#include <stdlib.h>

namespace rad
{
void* default_allocator_t::allocate(
    std::size_t size,
    std::size_t alignment)
{
    const auto ptr = _aligned_malloc(size, alignment);

    if (!ptr)
    {
        throw std::bad_alloc();
    }

    return ptr;
}

void* default_allocator_t::reallocate(
    void* ptr,
    std::size_t oldSize,
    std::size_t newSize,
    std::size_t alignment)
{
    const auto newPtr = _aligned_realloc(ptr, newSize, alignment);

    if (!newPtr)
    {
        throw std::bad_alloc();
    }

    return newPtr;
}

void default_allocator_t::free(void* ptr) noexcept
{
    _aligned_free(ptr);
}

default_allocator_t default_allocator{};
}
