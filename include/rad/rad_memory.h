/**
 * @file rad_memory.h
 * @author Graham Scott
 * @brief Header file providing memory allocation utilities.
 * @version 0.1
 * @date 2023-04-07
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 */
#ifndef RAD_MEMORY_H_INCLUDED
#define RAD_MEMORY_H_INCLUDED

#include "rad_base.h"
#include <cstddef>
#include <cstdint>

namespace rad
{
inline constexpr std::size_t default_alignment =
#ifdef _WIN32
    // Memory allocated on Windows using HeapAlloc is always aligned by 16.
    // alignof(std::max_align_t) on Windows using MSVC, however, is 8.
    16;
#else
    // This is accurate on all other platforms currently supported by libRad.
    alignof(std::max_align_t);
#endif

constexpr bool is_aligned(std::uintptr_t address, std::size_t alignment) noexcept
{
    return ((address % alignment) == 0);
}

constexpr bool is_aligned(const void* ptr, std::size_t alignment) noexcept
{
    return is_aligned(reinterpret_cast<std::uintptr_t>(ptr), alignment);
}

namespace detail_
{
RAD_API [[nodiscard]] void* allocate_(std::size_t size) noexcept;

RAD_API [[nodiscard]] void* reallocate_(void* ptr, std::size_t size) noexcept;

RAD_API void free_(void* ptr) noexcept;

RAD_API [[nodiscard]] void* allocate_aligned_(
    std::size_t size, std::size_t alignment) noexcept;

RAD_API [[nodiscard]] void* reallocate_aligned_(
    void* ptr, std::size_t size, std::size_t alignment) noexcept;

RAD_API void free_aligned_(void* ptr) noexcept;
} // detail_

struct debug_memory_alloc_info
{
    const char* filePath;
    unsigned int lineNumber;

    constexpr debug_memory_alloc_info(
        const char* filePath, unsigned int lineNumber) noexcept :
        filePath(filePath),
        lineNumber(lineNumber) {}
};

#if RAD_USE_DEBUG_MEMORY == 1
    namespace detail_
    {
        RAD_API [[nodiscard]] void* allocate_debug_(std::size_t size,
            debug_memory_alloc_info allocInfo) noexcept;
        
        RAD_API [[nodiscard]] void* reallocate_debug_(
            void* ptr, std::size_t size,
            debug_memory_alloc_info allocInfo) noexcept;

        RAD_API void free_debug_(void* ptr) noexcept;

        RAD_API [[nodiscard]] void* allocate_aligned_debug_(
            std::size_t size, std::size_t alignment,
            debug_memory_alloc_info allocInfo) noexcept;

        RAD_API [[nodiscard]] void* reallocate_aligned_debug_(
            void* ptr, std::size_t size, std::size_t alignment,
            debug_memory_alloc_info allocInfo) noexcept;

        RAD_API void free_aligned_debug_(void* ptr) noexcept;
    } // detail_

    #define RAD_GET_DEBUG_MEMORY_ALLOC_INFO()\
        ::rad::debug_memory_alloc_info(__FILE__, __LINE__)

    #define RAD_IF_DEBUG_MEMORY(...) __VA_ARGS__

    #define RAD_ALLOC_DEBUG(size, allocInfo)\
        ::rad::detail_::allocate_debug_((size), (allocInfo))

    #define RAD_ALLOC(size) RAD_ALLOC_DEBUG(\
        (size), RAD_GET_DEBUG_MEMORY_ALLOC_INFO())

    #define RAD_REALLOC_DEBUG(ptr, size, allocInfo)\
        ::rad::detail_::reallocate_debug_(\
            (ptr), (size), (allocInfo))

    #define RAD_REALLOC(ptr, size) RAD_REALLOC_DEBUG(\
        (ptr), (size), RAD_GET_DEBUG_MEMORY_ALLOC_INFO())

    #define RAD_FREE(ptr) ::rad::detail_::free_debug_((ptr))

    #define RAD_ALLOC_ALIGNED_DEBUG(size, alignment, allocInfo)\
        ::rad::detail_::allocate_aligned_debug_(\
            (size), (alignment), (allocInfo))

    #define RAD_ALLOC_ALIGNED(size, alignment) RAD_ALLOC_ALIGNED_DEBUG(\
        (size), (alignment), RAD_GET_DEBUG_MEMORY_ALLOC_INFO())

    #define RAD_REALLOC_ALIGNED_DEBUG(ptr, size, alignment, allocInfo)\
        ::rad::detail_::reallocate_aligned_debug_(\
            (ptr), (size), (alignment), (allocInfo))

    #define RAD_REALLOC_ALIGNED(ptr, size, alignment) RAD_REALLOC_ALIGNED_DEBUG(\
        (ptr), (size), (alignment), RAD_GET_DEBUG_MEMORY_ALLOC_INFO())

    #define RAD_FREE_ALIGNED(ptr) ::rad::detail_::free_aligned_debug_(ptr)

    #define RAD_NEW_DEBUG(type, allocInfo, ...)\
        new ((allocInfo), __VA_ARGS__) type

    #define RAD_NEW(type, ...) RAD_NEW_DEBUG(type,\
        RAD_GET_DEBUG_MEMORY_ALLOC_INFO(), __VA_ARGS__)
#else
    #define RAD_GET_DEBUG_MEMORY_ALLOC_INFO()

    #define RAD_IF_DEBUG_MEMORY(...)

    #define RAD_ALLOC_DEBUG(size, allocInfo)\
        ::rad::detail_::allocate_(size)

    #define RAD_ALLOC(size)\
        RAD_ALLOC_DEBUG((size), dummy_)

    #define RAD_REALLOC_DEBUG(ptr, size, allocInfo)\
        ::rad::detail_::reallocate_((ptr), (size))

    #define RAD_REALLOC(ptr, size)\
        RAD_REALLOC_DEBUG((ptr), (size), dummy_)

    #define RAD_FREE(ptr) ::rad::detail_::free_(ptr)

    #define RAD_ALLOC_ALIGNED_DEBUG(size, alignment, allocInfo)\
        ::rad::detail_::allocate_aligned_((size), (alignment))

    #define RAD_ALLOC_ALIGNED(size, alignment)\
        RAD_ALLOC_ALIGNED_DEBUG((size), (alignment), dummy_)

    #define RAD_REALLOC_ALIGNED_DEBUG(ptr, size, alignment, allocInfo)\
        ::rad::detail_::reallocate_aligned_((ptr), (size), (alignment))

    #define RAD_REALLOC_ALIGNED(ptr, size, alignment)\
        RAD_REALLOC_ALIGNED_DEBUG((ptr), (size), (alignment), dummy_)

    #define RAD_FREE_ALIGNED(ptr) ::rad::detail_::free_aligned_(ptr)

    #define RAD_NEW_DEBUG(type, allocInfo, ...)\
        new (__VA_ARGS__) type

    #define RAD_NEW(type, ...) RAD_NEW_DEBUG(type,\
        dummy_, __VA_ARGS__)
#endif
} // rad

#if RAD_USE_OPERATOR_NEW_DELETE_REPLACEMENTS == 1
    void* operator new(std::size_t count, rad::debug_memory_alloc_info allocInfo);

    void* operator new[](std::size_t count, rad::debug_memory_alloc_info allocInfo);

    void* operator new(std::size_t count, std::align_val_t al,
        rad::debug_memory_alloc_info allocInfo);

    void* operator new[](std::size_t count, std::align_val_t al,
        rad::debug_memory_alloc_info allocInfo);

    void operator delete(void* ptr, rad::debug_memory_alloc_info allocInfo) noexcept;

    void operator delete[](void* ptr, rad::debug_memory_alloc_info allocInfo) noexcept;

    void operator delete(void* ptr, std::align_val_t al,
        rad::debug_memory_alloc_info allocInfo) noexcept;

    void operator delete[](void* ptr, std::align_val_t al,
        rad::debug_memory_alloc_info allocInfo) noexcept;
#endif

#endif
