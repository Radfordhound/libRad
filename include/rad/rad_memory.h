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

inline bool is_aligned(const void* ptr, std::size_t alignment) noexcept
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
}

#if RAD_USE_DEBUG_MEMORY == 1
    struct debug_memory_alloc_info
    {
        const char* filePath;
        unsigned int lineNumber;

        constexpr debug_memory_alloc_info(
            const char* filePath, unsigned int lineNumber) noexcept
            : filePath(filePath)
            , lineNumber(lineNumber)
        {
        }
    };

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
    }

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
        new ((allocInfo), ## __VA_ARGS__) type

    #define RAD_NEW(type, ...) RAD_NEW_DEBUG(type,\
        RAD_GET_DEBUG_MEMORY_ALLOC_INFO(), ## __VA_ARGS__)
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

    /*
    * Okay, so if we just try to do this:
    *
    *   new (__VA_ARGS__) (type)
    * 
    * and the user calls the macro like this:
    * 
    *   RAD_NEW(int)
    * 
    * it would expand to the following:
    * 
    *   new () (type)
    * 
    * Unfortunately, those empty parenthesis in this context are
    * invalid, and this code will fail to compile!
    * 
    * So, we have to handle this case while still allowing the user
    * to optionally pass in variable arguments if they want to.
    * 
    * This macro is thus implemented with some crazy complicated magic that does just that.
    * It's hard to explain, but I'll try my best to give a good summary of how it works.
    * 
    * Basically, RAD_NEW_DEBUG_IMPL_HELPER2_ is where the magic happens.
    * It grabs just the first of the variable arguments given to us by the user,
    * and expands to the following:
    * 
    *   RAD_NEW_DEBUG_IMPL_HELPER3_ a ()
    * 
    * where a is the value of the first variable argument.
    * By itself, this is a meaningless value.
    * 
    * But, if the user passes ZERO variable arguments into the macro, then,
    * a will expand to NOTHING. This results in RAD_NEW_DEBUG_IMPL_HELPER2_
    * expanding to the following:
    * 
    *   RAD_NEW_DEBUG_IMPL_HELPER3_ ()
    * 
    * This is where the magic happens. You see, this will actually result in the
    * preprocessor "calling" RAD_NEW_DEBUG_IMPL_HELPER3_, thus expanding to the following:
    * 
    *   dummy_, new,
    * 
    * So basically, if we were given one or more variable arguments,
    * RAD_NEW_DEBUG_IMPL_HELPER2_ will expand to the one "meaningless" value.
    * 
    * Otherwise, if we were given zero variable arguments, it will
    * expand to two values.
    * 
    * This way, we can distinguish between having variable arguments and not.
    * 
    * The result of this expansion is "expanded" via RAD_NEW_DEBUG_IMPL_HELPER7_
    * to work around a quirk with some compilers (mainly MSVC), and then, is
    * passed into RAD_NEW_DEBUG_IMPL_HELPER4_, along with an extra argument:
    * 
    *   new (__VA_ARGS__)
    * 
    * RAD_NEW_DEBUG_IMPL_HELPER4_ then has to go through another macro -
    * RAD_NEW_DEBUG_IMPL_HELPER5_ - to work around yet another
    * quirk with some compilers (again, mainly MSVC).
    * 
    * Eventually, the result of all of this is passed into RAD_NEW_DEBUG_IMPL_HELPER6_,
    * which is the second part of the magic. It simply expands to whatever its second
    * argument is.
    * 
    * If there are one or more variable arguments, such as "a", "b", and "c",
    * it will have the following as its arguments:
    * 
    *   argument 1: RAD_NEW_DEBUG_IMPL_HELPER3_ a ()
    *   argument 2: new (a, b, c)
    *   argument 3: dummy_
    * 
    * Thus, it will end up expanding to:
    *   new (a, b, c)
    * 
    * Otherwise, if there are zero variable arguments,
    * it will have the following as its arguments:
    * 
    *   argument 1: dummy_
    *   argument 2: new
    *   argument 3: new ()
    * 
    * It will end up expanding to:
    *   new
    * 
    * From my testing, this approach works on all major compilers,
    * with any number of variable arguments, and is standards-compliant
    * enough to work even when compiled in pedantic mode.
    */
    #define RAD_NEW_DEBUG_IMPL_HELPER7_(a) a

    #define RAD_NEW_DEBUG_IMPL_HELPER6_(a, b, ...) b

    #define RAD_NEW_DEBUG_IMPL_HELPER5_(argsWithParenthesis) RAD_NEW_DEBUG_IMPL_HELPER6_ argsWithParenthesis

    #define RAD_NEW_DEBUG_IMPL_HELPER4_(...) RAD_NEW_DEBUG_IMPL_HELPER5_((__VA_ARGS__))

    #define RAD_NEW_DEBUG_IMPL_HELPER3_() dummy_, new,

    #define RAD_NEW_DEBUG_IMPL_HELPER2_(a, ...) RAD_NEW_DEBUG_IMPL_HELPER3_ a ()

    #define RAD_NEW_DEBUG_IMPL_HELPER1_(...) RAD_NEW_DEBUG_IMPL_HELPER7_(\
        RAD_NEW_DEBUG_IMPL_HELPER2_(__VA_ARGS__, dummy_)) // dummy_ on this line is just to
        // avoid compiler warnings that we didn't provide enough arguments.

    #define RAD_NEW_DEBUG(type, allocInfo, ...) RAD_NEW_DEBUG_IMPL_HELPER4_(\
        RAD_NEW_DEBUG_IMPL_HELPER1_(__VA_ARGS__), new (__VA_ARGS__), dummy_) type

    #define RAD_NEW(type, ...) RAD_NEW_DEBUG(type,\
        dummy_, ## __VA_ARGS__)
#endif
}

#if RAD_USE_OPERATOR_NEW_DELETE_REPLACEMENTS == 1 && RAD_USE_DEBUG_MEMORY == 1
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
