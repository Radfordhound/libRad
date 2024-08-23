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
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = pointer;
    using const_iterator = const_pointer;

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

    inline reference operator[](size_type index) const
    {
    #if RAD_USE_STRICT_BOUNDS_CHECKING == 1
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

    constexpr span(value_type &data) noexcept
        : data_(&data)
        , count_(1)
    {
    }
};
}

#endif
