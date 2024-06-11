/// @file rad_memory_win32.cpp
/// @author Graham Scott
/// @brief Windows implementation of memory allocation utilities.
/// @date 2023-04-07
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_memory.h"
#include "rad_memory_impl.h"
#include <cstdlib>
#include <crtdbg.h>

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
    RAD_VALIDATE_ALIGNED_ALLOC_ARGS(size, alignment)

    return _aligned_malloc(size, alignment);
}

void* reallocate_aligned_(void* ptr, std::size_t size, std::size_t alignment) noexcept
{
    RAD_VALIDATE_ALIGNED_ALLOC_ARGS(size, alignment)

    return _aligned_realloc(ptr, size, alignment);
}

void free_aligned_(void* ptr) noexcept
{
    _aligned_free(ptr);
}

#if RAD_USE_DEBUG_MEMORY == 1
    void* allocate_debug_(std::size_t size,
        debug_memory_alloc_info allocInfo) noexcept
    {
        return _malloc_dbg(size, _NORMAL_BLOCK,
            allocInfo.filePath, allocInfo.lineNumber);
    }

    void* reallocate_debug_(
        void* ptr, std::size_t size,
        debug_memory_alloc_info allocInfo) noexcept
    {
        return _realloc_dbg(ptr, size, _NORMAL_BLOCK,
            allocInfo.filePath, allocInfo.lineNumber);
    }

    void free_debug_(void* ptr) noexcept
    {
        _free_dbg(ptr, _NORMAL_BLOCK);
    }

    void* allocate_aligned_debug_(
        std::size_t size, std::size_t alignment,
        debug_memory_alloc_info allocInfo) noexcept
    {
        RAD_VALIDATE_ALIGNED_ALLOC_ARGS(size, alignment)

        return _aligned_malloc_dbg(size, alignment,
            allocInfo.filePath, allocInfo.lineNumber);
    }

    void* reallocate_aligned_debug_(
        void* ptr, std::size_t size, std::size_t alignment,
        debug_memory_alloc_info allocInfo) noexcept
    {
        RAD_VALIDATE_ALIGNED_ALLOC_ARGS(size, alignment)

        return _aligned_realloc_dbg(ptr, size, alignment,
            allocInfo.filePath, allocInfo.lineNumber);
    }

    void free_aligned_debug_(void* ptr) noexcept
    {
        _aligned_free_dbg(ptr);
    }
#endif
}
