/// @file rad_vector.h
/// @author Graham Scott
/// @brief Header file providing rad::vector.
/// @date 2023-06-14
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_VECTOR_H_INCLUDED
#define RAD_VECTOR_H_INCLUDED

#include "rad_base.h"
#include "rad_default_allocator.h"
#include "rad_memory.h"
#include <cstddef>
#include <cassert>
#include <type_traits>
#include <limits>
#include <algorithm>
#include <stdexcept>

namespace rad
{
/// @brief A resizable list of elements, like `std::vector`, but
/// with several additions/improvements.
///
/// @details Some of the changes include:
///
/// - Uses a `rad::allocator` for memory allocation instead of the STL allocator approach.
///
/// - Does reallocation through the `rad::allocator`, allowing for realloc optimizations
///   where applicable (e.g. growing/shrinking the memory block without a new allocation).
///
/// - Has a `release` function, which allows you to take ownership of the vector's buffer.
///
/// - Has a `no_value_init` variant of the default constructor, which allows for
///   default-construction of elements (instead of value-construction).
///
/// - Uses strict bounds checking in all indexing operations if
///   `RAD_USE_STRICT_BOUNDS_CHECKING` is defined to 1.
///
/// - Has a `no_value_init` variant of the push_back function, which allows for a
///   default-constructed element to be pushed back (instead of a value-constructed element).
///
/// @tparam T The value type of the vector.
template<typename T>
class vector
{
    using size_type_ = std::size_t;

    rad::allocator* allocator_ = &default_allocator;
    T*              dataBegin_ = nullptr;
    T*              dataEnd_ = nullptr;
    T*              bufEnd_ = nullptr;

    void destroy_data_() noexcept
    {
        // NOTE: This is safe even if dataBegin and dataEnd are both nullptr.
        allocator_->destroy(dataBegin_, size());
    }

    void reset_() noexcept
    {
        dataBegin_ = dataEnd_ = bufEnd_ = nullptr;
    }

    void validate_range_(size_type_ index) const
    {
        if (index >= size())
        {
            throw std::out_of_range(
                "The given index was outside of the vector's range"
            );
        }
    }

    size_type_ compute_new_capacity_(size_type_ newDataCount) const noexcept
    {
        // If geometric growth would exceed max_size() and potentially
        // overflow, we just return max_size() instead.
        const auto bufCount = capacity();
        const auto maxCount = max_size();

        if (bufCount > (maxCount - (bufCount / 2)))
        {
            return maxCount;
        }

        // Attempt to geometrically grow from current capacity,
        // falling back to newDataCount if the computed value is
        // not sufficient.
        return std::max<size_type_>(
            bufCount + (bufCount / 2),
            newDataCount
        );
    }

    void reallocate_(
        size_type_ oldCount,
        size_type_ newCapacity)
    {
        dataBegin_ = allocator_->resize(
            no_default_init,
            dataBegin_,
            oldCount,
            newCapacity
        );

        dataEnd_ = dataBegin_ + oldCount;
        bufEnd_ = dataBegin_ + newCapacity;
    }

    void reserve_for_one_()
    {
        if (dataEnd_ == bufEnd_)
        {
            const auto oldCount = size();
            const auto newCapacity = compute_new_capacity_(oldCount + 1);

            reallocate_(oldCount, newCapacity);
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

    static constexpr bool has_trivially_destructible_elements =
        std::is_trivially_destructible_v<T>;

    inline allocator_type& allocator() const noexcept
    {
        return *allocator_;
    }

    inline const T* data() const noexcept
    {
        return dataBegin_;
    }

    inline T* data() noexcept
    {
        return dataBegin_;
    }

    inline size_type size() const noexcept
    {
        return dataEnd_ - dataBegin_;
    }

    inline size_type capacity() const noexcept
    {
        return bufEnd_ - dataBegin_;
    }

    constexpr size_type max_size() const noexcept
    {
        return std::min<size_type>(
            (std::numeric_limits<difference_type>::max)(),
            (std::numeric_limits<size_type>::max)()
        );
    }

    [[nodiscard]] inline bool empty() const noexcept
    {
        return dataBegin_ == dataEnd_;
    }

    inline const_iterator cbegin() const noexcept
    {
        return dataBegin_;
    }

    inline const_iterator begin() const noexcept
    {
        return dataBegin_;
    }

    inline iterator begin() noexcept
    {
        return dataBegin_;
    }

    inline const_iterator cend() const noexcept
    {
        return dataEnd_;
    }

    inline const_iterator end() const noexcept
    {
        return dataEnd_;
    }

    inline iterator end() noexcept
    {
        return dataEnd_;
    }

    void reserve(size_type newCapacity)
    {
        const auto oldCapacity = capacity();

        if (newCapacity > oldCapacity)
        {
            reallocate_(size(), newCapacity);
        }
    }

    template<typename... Args>
    reference emplace_back(Args&&... args)
    {
        // Reallocate if necessary.
        reserve_for_one_();

        // Construct new element.
        ::new (dataEnd_) T(std::forward<Args>(args)...);
        return *(dataEnd_++);
    }

    reference push_back(no_value_init_t)
    {
        // Reallocate if necessary.
        reserve_for_one_();

        // Default-construct new element if necessary.
        if constexpr (!std::is_trivially_default_constructible_v<T>)
        {
            ::new (dataEnd_) T();
        }

        return *(dataEnd_++);
    }

    inline reference push_back(const T& val)
    {
        return emplace_back(val);
    }

    inline reference push_back(T&& val)
    {
        return emplace_back(std::move(val));
    }

    iterator erase(const_iterator pos)
    {
        const auto it = move_strong(
            const_cast<iterator>(pos + 1),
            dataEnd_,
            const_cast<iterator>(pos)
        );

        rad::destruct(*it);
        dataEnd_ = it;
        
        return const_cast<iterator>(pos);
    }

    // TODO: Range erase

    void pop_back()
    {
        assert(dataEnd_ != dataBegin_ &&
            "Cannot call pop_back() on an empty vector!"
        );

        rad::destruct(--dataEnd_);
    }

    void clear() noexcept
    {
        destroy_data_();
        reset_();
    }

    /// @brief Releases ownership of the data buffer
    /// to the caller and clears the vector.
    /// 
    /// @details After calling this function, it is the caller's
    /// responsibility to destruct the elements within the data buffer
    /// (unless `has_trivially_destructible_elements` is true), and to
    /// deallocate the memory using the allocator's free function
    /// (or equivalent).
    ///
    /// All of this can be done using the allocator's `destroy`
    /// helper function.
    /// 
    /// @return pointer A pointer to the data buffer, no longer owned by the vector.
    [[nodiscard]] pointer release() noexcept
    {
        const auto dataBuf = dataBegin_;
        reset_();
        return dataBuf;
    }

    const_reference at(size_type pos) const
    {
        validate_range_(pos);
        return dataBegin_[pos];
    }

    reference at(size_type pos)
    {
        validate_range_(pos);
        return dataBegin_[pos];
    }

    inline const_reference operator[](size_type pos) const
        noexcept(!RAD_USE_STRICT_BOUNDS_CHECKING)
    {
    #if RAD_USE_STRICT_BOUNDS_CHECKING
        validate_range_(pos);
    #endif

        return dataBegin_[pos];
    }

    inline reference operator[](size_type pos)
        noexcept(!RAD_USE_STRICT_BOUNDS_CHECKING)
    {
    #if RAD_USE_STRICT_BOUNDS_CHECKING
        validate_range_(pos);
    #endif

        return dataBegin_[pos];
    }

    // TODO: Handle propagate_on_container_copy_assignment properly!!!
    vector& operator=(const vector& other)
    {
        if (&other != this)
        {
            const auto otherCount = other.size();
            const auto newPtr = other.allocator_->create_copy<T>(
                other.dataBegin_,
                otherCount
            );

            clear();

            allocator_ = other.allocator_;
            dataBegin_ = newPtr;
            bufEnd_ = dataEnd_ = newPtr + otherCount;
        }
        
        return *this;
    }

    // TODO: Handle propagate_on_container_move_assignment properly!!!
    vector& operator=(vector&& other) noexcept
    {
        if (&other != this)
        {
            destroy_data_();

            allocator_ = other.allocator_;
            dataBegin_ = other.dataBegin_;
            dataEnd_ = other.dataEnd_;
            bufEnd_ = other.bufEnd_;

            other.reset_();
        }

        return *this;
    }

    constexpr vector() noexcept = default;

    constexpr explicit vector(rad::allocator& allocator) noexcept
        : allocator_(&allocator)
    {
    }

    explicit vector(no_value_init_t, size_type count)
        : dataBegin_(allocator_->create<T>(no_value_init, count))
        , dataEnd_(dataBegin_ + count)
        , bufEnd_(dataEnd_)
    {
    }

    explicit vector(no_value_init_t, rad::allocator& allocator, size_type count)
        : allocator_(&allocator)
        , dataBegin_(allocator_->create<T>(no_value_init, count))
        , dataEnd_(dataBegin_ + count)
        , bufEnd_(dataEnd_)
    {
    }

    template<typename... Args>
    explicit vector(size_type count, const Args&... args)
        : dataBegin_(allocator_->create<T>(count, args...))
        , dataEnd_(dataBegin_ + count)
        , bufEnd_(dataEnd_)
    {
    }

    template<typename... Args>
    explicit vector(rad::allocator& allocator, size_type count, const Args&... args)
        : allocator_(&allocator)
        , dataBegin_(allocator_->create<T>(count, args...))
        , dataEnd_(dataBegin_ + count)
        , bufEnd_(dataEnd_)
    {
    }

    vector(const vector& other)
        : allocator_(other.allocator_)
        , dataBegin_(allocator_->create_copy<T>(other.dataBegin_, other.size()))
        , dataEnd_(dataBegin_ + other.size())
        , bufEnd_(dataEnd_)
    {
    }

    constexpr vector(vector&& other) noexcept
        : allocator_(other.allocator_)
        , dataBegin_(other.dataBegin_)
        , dataEnd_(other.dataEnd_)
        , bufEnd_(other.bufEnd_)
    {
        other.reset_();
    }

    inline ~vector()
    {
        destroy_data_();
    }
};
}

#endif
