/// @file rad_allocator.h
/// @author Graham Scott
/// @brief Header file providing allocator base class.
/// @date 2024-12-05
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_ALLOCATOR_H_INCLUDED
#define RAD_ALLOCATOR_H_INCLUDED

#include "rad_base.h"
#include "rad_memory.h"
#include <cstddef>
#include <type_traits>
#include <new>

namespace rad
{
class allocator
{
public:
    virtual ~allocator() = default;

    virtual void* allocate(
        std::size_t size,
        std::size_t alignment
    ) = 0;

    virtual void* reallocate(
        void* ptr,
        std::size_t size,
        std::size_t alignment
    ) = 0;

    virtual void free(void* ptr) noexcept = 0;

    template<typename T>
    T* create(no_value_init_t, std::size_t count)
    {
        const auto ptr = static_cast<T*>(allocate(
            sizeof(T) * count,
            alignof(T)
        ));
        
        if constexpr (!std::is_trivially_default_constructible_v<T>)
        {
            try
            {
                std::uninitialized_default_construct(ptr, ptr + count);
            }
            catch (...)
            {
                free(ptr);
                throw;
            }
        }

        return ptr;
    }

    template<typename T, typename... Args>
    T* create(std::size_t count, const Args&... args)
    {
        const auto ptr = static_cast<T*>(allocate(
            sizeof(T) * count,
            alignof(T)
        ));
        
        try
        {
            uninitialized_direct_construct(ptr, ptr + count, args...);
        }
        catch (...)
        {
            free(ptr);
            throw;
        }

        return ptr;
    }

    template<typename T>
    T* create_copy(const T* ptr, std::size_t count)
    {
        const auto newPtr = static_cast<T*>(allocate(
            sizeof(T) * count,
            alignof(T)
        ));

        try
        {
            std::uninitialized_copy(ptr, ptr + count, newPtr);
        }
        catch (...)
        {
            free(newPtr);
            throw;
        }

        return newPtr;
    }

    template<typename T>
    T* resize(no_default_init_t, T* ptr, std::size_t oldCount, std::size_t newCount)
    {
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            // NOTE: trivially-copyable types must have trivial destructors
            // and must not have any non-trivial copy/move constructors/assignment
            // operators; so they are safe to perform bitwise-copies on.
            return static_cast<T*>(reallocate(ptr, sizeof(T) * newCount, alignof(T)));
        }
        else
        {
            // Allocate new memory.
            const auto newPtr = static_cast<T*>(allocate(
                sizeof(T) * newCount,
                alignof(T)
            ));

            const auto minCount = (newCount > oldCount) ? oldCount : newCount;

            // Move old elements into new memory.
            try
            {
                uninitialized_move_strong(ptr, ptr + minCount, newPtr);
            }
            catch (...)
            {
                free(newPtr);
                throw;
            }

            // Destruct the old elements.
            destruct(ptr, ptr + oldCount);

            // Free the old memory and return new memory.
            free(ptr);
            return newPtr;
        }
    }

    template<typename T>
    T* resize(no_value_init_t, T* ptr, std::size_t oldCount, std::size_t newCount)
    {
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            // NOTE: trivially-copyable types must have trivial destructors
            // and must not have any non-trivial copy/move constructors/assignment
            // operators; so they are safe to perform bitwise-copies on.
            return static_cast<T*>(reallocate(ptr, sizeof(T) * newCount, alignof(T)));
        }
        else
        {
            // Allocate new memory.
            const auto newPtr = static_cast<T*>(allocate(
                sizeof(T) * newCount,
                alignof(T)
            ));

            const auto minCount = (newCount > oldCount) ? oldCount : newCount;

            // Move old elements into new memory.
            try
            {
                uninitialized_move_strong(ptr, ptr + minCount, newPtr);
            }
            catch (...)
            {
                free(newPtr);
                throw;
            }

            // Default-construct extra new elements, if any.
            if constexpr (!std::is_trivially_default_constructible_v<T>)
            {
                if (newCount > oldCount)
                {
                    try
                    {
                        std::uninitialized_default_construct(
                            newPtr + minCount,
                            newPtr + newCount
                        );
                    }
                    catch (...)
                    {
                        destruct(newPtr, newPtr + minCount);
                        free(newPtr);
                        throw;
                    }
                }
            }

            // Destruct the old elements.
            destruct(ptr, ptr + oldCount);

            // Free the old memory and return new memory.
            free(ptr);
            return newPtr;
        }
    }

    template<typename T>
    T* resize(T* ptr, std::size_t oldCount, std::size_t newCount)
    {
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            // NOTE: trivially-copyable types must have trivial destructors
            // and must not have any non-trivial copy/move constructors/assignment
            // operators; so they are safe to perform bitwise-copies on.
            return static_cast<T*>(reallocate(ptr, sizeof(T) * newCount, alignof(T)));
        }
        else
        {
            // Allocate new memory.
            const auto newPtr = static_cast<T*>(allocate(
                sizeof(T) * newCount,
                alignof(T)
            ));

            const auto minCount = (newCount > oldCount) ? oldCount : newCount;

            // Move old elements into new memory.
            try
            {
                uninitialized_move_strong(ptr, ptr + minCount, newPtr);
            }
            catch (...)
            {
                free(newPtr);
                throw;
            }

            // Value-construct extra new elements, if any.
            if (newCount > oldCount)
            {
                try
                {
                    std::uninitialized_value_construct(
                        newPtr + minCount,
                        newPtr + newCount
                    );
                }
                catch (...)
                {
                    destruct(newPtr, newPtr + minCount);
                    free(newPtr);
                    throw;
                }
            }

            // Destruct the old elements.
            destruct(ptr, ptr + oldCount);

            // Free the old memory and return new memory.
            free(ptr);
            return newPtr;
        }
    }

    template<typename T>
    void destroy(T* ptr) noexcept
    {
        destruct(*ptr);
        free(ptr);
    }

    template<typename T>
    void destroy(T* ptr, std::size_t count) noexcept
    {
        destruct(ptr, ptr + count);
        free(ptr);
    }
};

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
