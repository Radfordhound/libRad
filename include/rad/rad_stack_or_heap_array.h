/// @file rad_stack_or_heap_array.h
/// @author Graham Scott
/// @brief Header file providing rad::stack_or_heap_array.
/// @date 2023-06-05
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_STACK_OR_HEAP_ARRAY_H_INCLUDED
#define RAD_STACK_OR_HEAP_ARRAY_H_INCLUDED

#include "rad_base.h"
#include "rad_allocator.h"
#include "rad_memory.h"
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <initializer_list>

namespace rad
{
template<typename T, std::size_t MaxStackCount>
class stack_or_heap_array
{
    using size_type_ = std::size_t;

    rad::allocator*     allocator_ = &default_allocator;
    size_type_          count_ = 0;
    T*                  data_;

    union
    {
        T               stackData_[MaxStackCount];
        char            dummy_;
    };

    void validate_range_(size_type_ index) const
    {
        if (index >= count_)
        {
            throw std::out_of_range(
                "The given index was outside of the "
                "stack_or_heap_array's range"
            );
        }
    }

    void free_() noexcept
    {
        if (is_heap())
        {
            allocator_->free(data_);
        }
    }

    T* allocate_(size_type_ count)
    {
        if (count > MaxStackCount)
        {
            return static_cast<T*>(allocator_->allocate(
                sizeof(T) * count,
                alignof(T)
            ));
        }
        else
        {
            return stackData_;
        }
    }

    T* reallocate_(size_type_ count)
    {
        const auto oldCount = count_;
        count_ = 0;

        rad::destruct(begin(), end());

        if (is_heap())
        {
            return static_cast<T*>(allocator_->reallocate(
                data_,
                sizeof(T) * oldCount,
                sizeof(T) * count,
                alignof(T)
            ));
        }
        else
        {
            return allocate_(count);
        }
    }

    void destroy_data_() noexcept
    {
        rad::destruct(begin(), end());
        free_();
    }

    void copy_data_(const stack_or_heap_array& other)
    {
        data_ = allocate_(other.count_);

        try
        {
            std::uninitialized_copy(other.begin(), other.end(), data_);
        }
        catch (...)
        {
            free_();
            throw;
        }
    }

public:
    using value_type            = T;
    using allocator_type        = rad::allocator;
    using size_type             = size_type_;
    using difference_type       = std::ptrdiff_t;
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using pointer               = value_type*;
    using const_pointer         = const value_type*;
    using iterator              = pointer;
    using const_iterator        = const_pointer;
    // TODO: reverse_iterator
    // TODO: const_reverse_iterator

    constexpr allocator_type& allocator() const noexcept
    {
        return *allocator_;
    }

    inline std::size_t size() const noexcept
    {
        return count_;
    }

    [[nodiscard]] inline bool empty() const noexcept
    {
        return count_ == 0;
    }

    inline const T* data() const noexcept
    {
        return data_;
    }

    inline T* data() noexcept
    {
        return data_;
    }

    inline bool is_heap() const noexcept
    {
        return data_ != stackData_;
    }

    inline const T* begin() const noexcept
    {
        return data_;
    }

    inline T* begin() noexcept
    {
        return data_;
    }

    inline const T* end() const noexcept
    {
        return data_ + count_;
    }

    inline T* end() noexcept
    {
        return data_ + count_;
    }

    void clear() noexcept
    {
        destroy_data_();

        count_ = 0;
        data_ = stackData_;
    }

    void assign(no_value_init_t, size_type count)
    {
        data_ = reallocate_(count);

        std::uninitialized_default_construct(data_, data_ + count);
        count_ = count;
    }

    template<typename... Args>
    void assign(size_type count, const Args&... args)
    {
        data_ = reallocate_(count);

        uninitialized_direct_construct(data_, data_ + count, args...);
        count_ = count;
    }

    // TODO: Add other overloads of assign.

    inline const T& operator[](std::size_t index) const
        noexcept(!RAD_USE_STRICT_BOUNDS_CHECKING)
    {
    #if RAD_USE_STRICT_BOUNDS_CHECKING
        validate_range_(index);
    #endif

        return data_[index];
    }

    inline T& operator[](std::size_t index)
        noexcept(!RAD_USE_STRICT_BOUNDS_CHECKING)
    {
    #if RAD_USE_STRICT_BOUNDS_CHECKING
        validate_range_(index);
    #endif

        return data_[index];
    }

    stack_or_heap_array& operator=(const stack_or_heap_array& other)
    {
        if (&other != this)
        {
            clear();

            allocator_ = other.allocator_;

            copy_data_(other);
            count_ = other.count_;
        }

        return *this;
    }

    stack_or_heap_array& operator=(stack_or_heap_array&& other)
        noexcept(noexcept(
            uninitialized_move_strong(
                std::declval<pointer>(),
                std::declval<pointer>(),
                std::declval<pointer>()
            )
        ))
    {
        if (&other != this)
        {
            clear();

            if (other.is_heap())
            {
                data_ = other.data_;
                other.data_ = other.stackData_;
            }
            else
            {
                // NOTE: data_ is already set to stackData_ here due to the clear call above.
                uninitialized_move_strong(other.begin(), other.end(), stackData_);
                rad::destruct(other.begin(), other.end());
            }

            allocator_ = other.allocator_;
            count_ = other.count_;
            other.count_ = 0;
        }

        return *this;
    }

    stack_or_heap_array() noexcept
        : data_(stackData_)
    {
    }

    explicit stack_or_heap_array(rad::allocator& allocator) noexcept
        : allocator_(&allocator)
        , data_(stackData_)
    {
    }

    explicit stack_or_heap_array(
        no_value_init_t,
        rad::allocator& allocator,
        std::size_t count)
        : allocator_(&allocator)
        , count_(count)
        , data_(allocate_(count))
    {
        try
        {
            std::uninitialized_default_construct(begin(), end());
        }
        catch (...)
        {
            free_();
            throw;
        }
    }

    explicit stack_or_heap_array(no_value_init_t, std::size_t count)
        : count_(count)
        , data_(allocate_(count))
    {
        try
        {
            std::uninitialized_default_construct(begin(), end());
        }
        catch (...)
        {
            free_();
            throw;
        }
    }

    template<typename... Args>
    explicit stack_or_heap_array(
        rad::allocator& allocator,
        size_type count,
        const Args&... args)
        : allocator_(&allocator)
        , count_(count)
        , data_(allocate_(count))
    {
        try
        {
            uninitialized_direct_construct(begin(), end(), args...);
        }
        catch (...)
        {
            free_();
            throw;
        }
    }

    template<typename... Args>
    explicit stack_or_heap_array(size_type count, const Args&... args)
        : count_(count)
        , data_(allocate_(count))
    {
        try
        {
            uninitialized_direct_construct(begin(), end(), args...);
        }
        catch (...)
        {
            free_();
            throw;
        }
    }

    template<typename InputIt>
    stack_or_heap_array(rad::allocator& allocator, InputIt begin, InputIt end)
        : allocator_(&allocator)
        , count_(std::distance(begin, end))
        , data_(allocate_(count_))
    {
        try
        {
            std::uninitialized_copy(begin, end, data_);
        }
        catch (...)
        {
            free_();
            throw;
        }
    }

    template<typename InputIt>
    stack_or_heap_array(InputIt begin, InputIt end)
        : count_(std::distance(begin, end))
        , data_(allocate_(count_))
    {
        try
        {
            std::uninitialized_copy(begin, end, data_);
        }
        catch (...)
        {
            free_();
            throw;
        }
    }

    stack_or_heap_array(rad::allocator& allocator, std::initializer_list<T> ilist)
        : stack_or_heap_array(allocator, ilist.begin(), ilist.end())
    {
    }

    stack_or_heap_array(std::initializer_list<T> ilist)
        : stack_or_heap_array(ilist.begin(), ilist.end())
    {
    }

    stack_or_heap_array(const stack_or_heap_array& other)
        : allocator_(other.allocator_)
        , count_(other.count_)
    {
        copy_data_(other);
    }

    stack_or_heap_array(stack_or_heap_array&& other)
        noexcept(noexcept(
            uninitialized_move_strong(
                std::declval<pointer>(),
                std::declval<pointer>(),
                std::declval<pointer>()
            )
        ))

        : allocator_(other.allocator_)
        , count_(other.count_)
    {
        if (other.is_heap())
        {
            data_ = other.data_;
            other.data_ = other.stackData_;
        }
        else
        {
            data_ = stackData_;
            uninitialized_move_strong(other.begin(), other.end(), stackData_);
            rad::destruct(other.begin(), other.end());
        }

        other.count_ = 0;
    }

    inline ~stack_or_heap_array()
    {
        destroy_data_();
    }
};
}

#endif
