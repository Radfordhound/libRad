/**
 * @file rad_memory_impl.cpp
 * @author Graham Scott
 * @brief Common implementation file for libRad memory utilities.
 * @version 0.1
 * @date 2023-04-08
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 * 
 */
#include "rad_memory.h"

#if RAD_USE_OPERATOR_NEW_DELETE_REPLACEMENTS == 1

void* operator new(std::size_t count)
{
    const auto ptr = RAD_ALLOC(count);
    if (!ptr)
    {
        throw std::bad_alloc();
    }

    return ptr;
}

void* operator new[](std::size_t count)
{
    const auto ptr = RAD_ALLOC(count);
    if (!ptr)
    {
        throw std::bad_alloc();
    }

    return ptr;
}

void* operator new(std::size_t count, std::align_val_t al)
{
    const auto ptr = RAD_ALLOC_ALIGNED(count, static_cast<std::size_t>(al));
    if (!ptr)
    {
        throw std::bad_alloc();
    }

    return ptr;
}

void* operator new[](std::size_t count, std::align_val_t al)
{
    const auto ptr = RAD_ALLOC_ALIGNED(count, static_cast<std::size_t>(al));
    if (!ptr)
    {
        throw std::bad_alloc();
    }

    return ptr;
}

void* operator new(std::size_t count, const std::nothrow_t&) noexcept
{
    return RAD_ALLOC(count);
}

void* operator new[](std::size_t count, const std::nothrow_t&) noexcept
{
    return RAD_ALLOC(count);
}

void* operator new(std::size_t count,
    std::align_val_t al, const std::nothrow_t&) noexcept
{
    return RAD_ALLOC_ALIGNED(count, static_cast<std::size_t>(al));
}

void* operator new[](std::size_t count,
    std::align_val_t al, const std::nothrow_t&) noexcept
{
    return RAD_ALLOC_ALIGNED(count, static_cast<std::size_t>(al));
}

void operator delete(void* ptr) noexcept
{
    RAD_FREE(ptr);
}

void operator delete[](void* ptr) noexcept
{
    RAD_FREE(ptr);
}

void operator delete(void* ptr, std::align_val_t al) noexcept
{
    RAD_FREE_ALIGNED(ptr);
}

void operator delete[](void* ptr, std::align_val_t al) noexcept
{
    RAD_FREE_ALIGNED(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept
{
    RAD_FREE(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept
{
    RAD_FREE(ptr);
}

void operator delete(void* ptr, std::align_val_t al,
    const std::nothrow_t&) noexcept
{
    RAD_FREE_ALIGNED(ptr);
}

void operator delete[](void* ptr, std::align_val_t al,
    const std::nothrow_t&) noexcept
{
    RAD_FREE_ALIGNED(ptr);
}

#if RAD_USE_DEBUG_MEMORY == 1
    void* operator new(std::size_t count, rad::debug_memory_alloc_info allocInfo)
    {
        const auto ptr = RAD_ALLOC_DEBUG(count, allocInfo);
        if (!ptr)
        {
            throw std::bad_alloc();
        }

        return ptr;
    }

    void* operator new[](std::size_t count, rad::debug_memory_alloc_info allocInfo)
    {
        const auto ptr = RAD_ALLOC_DEBUG(count, allocInfo);
        if (!ptr)
        {
            throw std::bad_alloc();
        }

        return ptr;
    }

    void* operator new(std::size_t count, std::align_val_t al,
        rad::debug_memory_alloc_info allocInfo)
    {
        const auto ptr = RAD_ALLOC_ALIGNED_DEBUG(count,
            static_cast<std::size_t>(al), allocInfo);

        if (!ptr)
        {
            throw std::bad_alloc();
        }

        return ptr;
    }

    void* operator new[](std::size_t count, std::align_val_t al,
        rad::debug_memory_alloc_info allocInfo)
    {
        const auto ptr = RAD_ALLOC_ALIGNED_DEBUG(count,
            static_cast<std::size_t>(al), allocInfo);

        if (!ptr)
        {
            throw std::bad_alloc();
        }

        return ptr;
    }

    void operator delete(void* ptr, rad::debug_memory_alloc_info allocInfo) noexcept
    {
        RAD_FREE(ptr);
    }

    void operator delete[](void* ptr, rad::debug_memory_alloc_info allocInfo) noexcept
    {
        RAD_FREE(ptr);
    }

    void operator delete(void* ptr, std::align_val_t al,
        rad::debug_memory_alloc_info allocInfo) noexcept
    {
        RAD_FREE_ALIGNED(ptr);
    }

    void operator delete[](void* ptr, std::align_val_t al,
        rad::debug_memory_alloc_info allocInfo) noexcept
    {
        RAD_FREE_ALIGNED(ptr);
    }
#endif

#endif
