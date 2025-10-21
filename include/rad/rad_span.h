/// @file rad_span.h
/// @author Graham Scott
/// @brief Header file providing rad::span, a class which represents
/// a range (or "span") or elements of a given type.
/// @date 2024-07-12
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_SPAN_H_INCLUDED
#define RAD_SPAN_H_INCLUDED

#include "rad_base.h"
#include <cstddef>
#include <cassert>
#include <stdexcept>

namespace rad
{
template<typename T>
class span
{
    T*              data_ = nullptr;
    std::size_t     count_ = 0;

    void validate_range_(std::size_t index) const
    {
        if (index >= count_)
        {
            throw std::out_of_range(
                "The given index was outside of the span's range"
            );
        }
    }

public:
    using element_type      = T;
    using value_type        = T;
    using pointer           = T*;
    using const_pointer     = const T*;
    using reference         = T&;
    using const_reference   = const T&;
    using size_type         = std::size_t;
    using difference_type   = std::ptrdiff_t;
    using iterator          = pointer;
    using const_iterator    = const_pointer;

    constexpr pointer data() const noexcept
    {
        return data_;
    }

    constexpr size_type size() const noexcept
    {
        return count_;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return count_ == 0;
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return data_;
    }

    constexpr const_iterator cend() const noexcept
    {
        return data_ + count_;
    }

    constexpr iterator begin() const noexcept
    {
        return data_;
    }

    constexpr iterator end() const noexcept
    {
        return data_ + count_;
    }

    inline reference front() const noexcept
    {
        return *data_;
    }

    inline reference back() const noexcept
    {
        return data_[count_ - 1];
    }

    reference at(size_type index) const
    {
        validate_range_(index);
        return data_[index];
    }

    span<element_type> slice_unchecked(
        size_type offset,
        size_type count = -1) const noexcept
    {
        if (count == -1)
        {
            count = (count_ - offset);
        }

        return { data_ + offset, count };
    }

    span<element_type> slice(
        size_type offset,
        size_type count = -1) const
    {
        validate_range_(offset);

        if (count == -1)
        {
            count = (count_ - offset);
        }
        else if (count > (count_ - offset))
        {
            throw std::out_of_range(
                "The requested slice's range was outside of the span's range"
            );
        }

        return { data_ + offset, count };
    }

    inline span<element_type> subspan(
        size_type offset,
        size_type count = -1) const
    {
        return slice(offset, count);
    }

    constexpr span<element_type> slice_range_unchecked(
        size_type beginIndex,
        size_type endIndex) const noexcept
    {
        return { data_ + beginIndex, endIndex - beginIndex };
    }

    span<element_type> slice_range(
        size_type beginIndex,
        size_type endIndex) const
    {
        assert(endIndex >= beginIndex &&
            "The given beginIndex must be <= the given endIndex"
        );

        validate_range_(beginIndex);

        if (endIndex > count_)
        {
            throw std::out_of_range(
                "The requested slice's range was outside of the span's range"
            );
        }

        return slice_range_unchecked(beginIndex, endIndex);
    }

    constexpr explicit operator bool() const noexcept
    {
        return data_ != nullptr;
    }

    inline reference operator[](size_type index) const
        noexcept(!RAD_USE_STRICT_BOUNDS_CHECKING)
    {
    #if RAD_USE_STRICT_BOUNDS_CHECKING
        validate_range_(index);
    #endif

        return data_[index];
    }

    constexpr span() noexcept = default;

    constexpr span(std::nullptr_t) noexcept
    {
    }

    constexpr span(pointer data, size_type count) noexcept
        : data_(data)
        , count_(count)
    {
    }

    template<std::size_t Count>
    constexpr span(value_type (&data)[Count]) noexcept
        : data_(data)
        , count_(Count)
    {
    }

    constexpr span(value_type& data) noexcept
        : data_(&data)
        , count_(1)
    {
    }

    template<typename ContainerType>
    constexpr span(const ContainerType& container)
        noexcept(noexcept(container.data()) && noexcept(container.size()))
        : data_(container.data())
        , count_(container.size())
    {
    }

    template<typename ContainerType>
    constexpr span(ContainerType& container)
        noexcept(noexcept(container.data()) && noexcept(container.size()))
        : data_(container.data())
        , count_(container.size())
    {
    }
};
}

#endif
