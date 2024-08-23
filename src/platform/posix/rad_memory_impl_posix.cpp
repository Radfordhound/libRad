/// @file rad_memory_impl_posix.cpp
/// @author Graham Scott
/// @brief POSIX implementation of rad_memory.h
/// @date 2023-04-07
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_memory.h"
#include "../../rad_memory_impl.h"
#include <cstdlib>

namespace rad::detail_
{
void* allocate_(std::size_t size) noexcept
{
    return std::malloc(size);
}

void* reallocate_(void* ptr, std::size_t size) noexcept
{
    return std::realloc(ptr, size);
}

void free_(void* ptr) noexcept
{
    std::free(ptr);
}

void* allocate_aligned_(std::size_t size, std::size_t alignment) noexcept
{
    return std::aligned_alloc(alignment, size);
}

void* reallocate_aligned_(void* ptr, std::size_t size, std::size_t alignment) noexcept
{
    return nullptr; // TODO !!!
}

void free_aligned_(void* ptr) noexcept
{
    std::free(ptr);
}

#if RAD_USE_DEBUG_MEMORY == 1
    void* allocate_debug_(std::size_t size,
        debug_memory_alloc_info allocInfo) noexcept
    {
        return std::malloc(size);
    }

    void* reallocate_debug_(
        void* ptr, std::size_t size,
        debug_memory_alloc_info allocInfo) noexcept
    {
        return std::realloc(ptr, size);
    }

    void free_debug_(void* ptr) noexcept
    {
        std::free(ptr);
    }

    void* allocate_aligned_debug_(
        std::size_t size, std::size_t alignment,
        debug_memory_alloc_info allocInfo) noexcept
    {
        return std::aligned_alloc(alignment, size);
    }

    void* reallocate_aligned_debug_(
        void* ptr, std::size_t size, std::size_t alignment,
        debug_memory_alloc_info allocInfo) noexcept
    {
        return nullptr; // TODO !!!
    }

    void free_aligned_debug_(void* ptr) noexcept
    {
        std::free(ptr);
    }
#endif
}
