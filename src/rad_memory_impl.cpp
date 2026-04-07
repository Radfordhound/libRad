/// @file rad_memory_impl.cpp
/// @author Graham Scott
/// @brief Common implementation file for libRad memory utilities.
/// @date 2023-04-08
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_allocator.h"

#if RAD_USE_OPERATOR_NEW_DELETE_REPLACEMENTS

void* operator new(std::size_t count)
{
    return rad::default_allocator.allocate(
        count,
        __STDCPP_DEFAULT_NEW_ALIGNMENT__
    );
}

void* operator new[](std::size_t count)
{
    return rad::default_allocator.allocate(
        count,
        __STDCPP_DEFAULT_NEW_ALIGNMENT__
    );
}

void* operator new(std::size_t count, std::align_val_t al)
{
    return rad::default_allocator.allocate(
        count,
        static_cast<std::size_t>(al)
    );
}

void* operator new[](std::size_t count, std::align_val_t al)
{
    return rad::default_allocator.allocate(
        count,
        static_cast<std::size_t>(al)
    );
}

void* operator new(std::size_t count, const std::nothrow_t&) noexcept
{
    try
    {
        return rad::default_allocator.allocate(
            count,
            __STDCPP_DEFAULT_NEW_ALIGNMENT__
        );
    }
    catch (...)
    {
        return nullptr;
    }
}

void* operator new[](std::size_t count, const std::nothrow_t&) noexcept
{
    try
    {
        return rad::default_allocator.allocate(
            count,
            __STDCPP_DEFAULT_NEW_ALIGNMENT__
        );
    }
    catch (...)
    {
        return nullptr;
    }
}

void* operator new(std::size_t count,
    std::align_val_t al, const std::nothrow_t&) noexcept
{
    try
    {
        return rad::default_allocator.allocate(
            count,
            static_cast<std::size_t>(al)
        );
    }
    catch (...)
    {
        return nullptr;
    }
}

void* operator new[](std::size_t count,
    std::align_val_t al, const std::nothrow_t&) noexcept
{
    try
    {
        return rad::default_allocator.allocate(
            count,
            static_cast<std::size_t>(al)
        );
    }
    catch (...)
    {
        return nullptr;
    }
}

void operator delete(void* ptr) noexcept
{
    rad::default_allocator.free(ptr);
}

void operator delete[](void* ptr) noexcept
{
    rad::default_allocator.free(ptr);
}

void operator delete(void* ptr, std::align_val_t al) noexcept
{
    rad::default_allocator.free(ptr);
}

void operator delete[](void* ptr, std::align_val_t al) noexcept
{
    rad::default_allocator.free(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept
{
    rad::default_allocator.free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept
{
    rad::default_allocator.free(ptr);
}

void operator delete(void* ptr, std::align_val_t al,
    const std::nothrow_t&) noexcept
{
    rad::default_allocator.free(ptr);
}

void operator delete[](void* ptr, std::align_val_t al,
    const std::nothrow_t&) noexcept
{
    rad::default_allocator.free(ptr);
}

#endif
