/// @file rad_pair.h
/// @author Graham Scott
/// @brief Header file providing rad::pair; a class similar to
/// std::pair, but with additional features/optimizations.
/// @date 2023-04-04
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_PAIR_H_INCLUDED
#define RAD_PAIR_H_INCLUDED

#include <type_traits>

namespace rad
{
namespace detail_
{
    template<typename T1, typename T2>
    constexpr bool is_default_constructible_v = (
        std::is_default_constructible_v<T1> &&
        std::is_default_constructible_v<T2>);

    template<typename T1, typename T2>
    constexpr bool is_nothrow_default_constructible_v = (
        std::is_nothrow_default_constructible_v<T1> &&
        std::is_nothrow_default_constructible_v<T2>);

    template<typename T1, typename T2,
        bool HasT1 = !std::is_empty_v<T1> || std::is_final_v<T1>,
        bool HasT2 = !std::is_empty_v<T1> || std::is_final_v<T1>>
    class compressed_pair final
    {
        T1 first_;
        T2 second_;

    public:
        constexpr const T1& first() const noexcept
        {
            return first_;
        }

        constexpr const T2& second() const noexcept
        {
            return second_;
        }

        constexpr T1& first() noexcept
        {
            return first_;
        }

        constexpr T2& second() noexcept
        {
            return second_;
        }

        template<std::enable_if_t<is_default_constructible_v<T1, T2>, int> = 0>
        constexpr compressed_pair() noexcept(
            is_nothrow_default_constructible_v<T1, T2>)
        {
        }

        // TODO: Mark as noexcept if possible.
        template<typename U1 = T1, typename U2 = T2, std::enable_if_t<
            (std::is_copy_constructible_v<U1> && std::is_copy_constructible_v<U2>),
            int> = 0>
        constexpr compressed_pair(const T1& a, const T2& b)
            : first_(a)
            , second_(b)
        {
        }
    };

    template<typename T1, typename T2>
    class compressed_pair<T1, T2, false, true> final : private T1
    {
        T2 second_;

    public:
        constexpr const T1& first() const noexcept
        {
            return *this;
        }

        constexpr const T2& second() const noexcept
        {
            return second_;
        }

        constexpr T1& first() noexcept
        {
            return *this;
        }

        constexpr T2& second() noexcept
        {
            return second_;
        }

        template<std::enable_if_t<is_default_constructible_v<T1, T2>, int> = 0>
        constexpr compressed_pair() noexcept(
            is_nothrow_default_constructible_v<T1, T2>)
        {
        }

        // TODO: Mark as noexcept if possible.
        template<typename U1 = T1, typename U2 = T2, std::enable_if_t<
            (std::is_copy_constructible_v<U1> && std::is_copy_constructible_v<U2>),
            int> = 0>
        constexpr compressed_pair(const T1& a, const T2& b)
            : T1(a)
            , second_(b)
        {
        }
    };

    template<typename T1, typename T2>
    class compressed_pair<T1, T2, true, false> final : private T2
    {
        T1 first_;

    public:
        constexpr const T1& first() const noexcept
        {
            return first_;
        }

        constexpr const T2& second() const noexcept
        {
            return *this;
        }

        constexpr T1& first() noexcept
        {
            return first_;
        }

        constexpr T2& second() noexcept
        {
            return *this;
        }

        template<std::enable_if_t<is_default_constructible_v<T1, T2>, int> = 0>
        constexpr compressed_pair() noexcept(
            is_nothrow_default_constructible_v<T1, T2>)
        {
        }

        // TODO: Mark as noexcept if possible.
        template<typename U1 = T1, typename U2 = T2, std::enable_if_t<
            (std::is_copy_constructible_v<U1> && std::is_copy_constructible_v<U2>),
            int> = 0>
        constexpr compressed_pair(const T1& a, const T2& b)
            : T2(a)
            , first_(b)
        {
        }
    };

    template<typename T1, typename T2>
    class compressed_pair<T1, T2, false, false> final : private T1, private T2
    {
    public:
        constexpr const T1& first() const noexcept
        {
            return *this;
        }

        constexpr const T2& second() const noexcept
        {
            return *this;
        }

        constexpr T1& first() noexcept
        {
            return *this;
        }

        constexpr T2& second() noexcept
        {
            return *this;
        }

        template<std::enable_if_t<is_default_constructible_v<T1, T2>, int> = 0>
        constexpr compressed_pair() noexcept(
            is_nothrow_default_constructible_v<T1, T2>)
        {
        }

        // TODO: Mark as noexcept if possible.
        template<typename U1 = T1, typename U2 = T2, std::enable_if_t<
            (std::is_copy_constructible_v<U1> && std::is_copy_constructible_v<U2>),
            int> = 0>
        constexpr compressed_pair(const T1& a, const T2& b)
            : T1(a)
            , T2(b)
        {
        }
    };
} // detail_

template<typename T1, typename T2>
class pair
{
    detail_::compressed_pair<T1, T2> data_;

public:
    using first_type    = T1;
    using second_type   = T2;

    constexpr const T1& first() const noexcept
    {
        return data_.first();
    }

    constexpr const T2& second() const noexcept
    {
        return data_.second();
    }

    constexpr T1& first() noexcept
    {
        return data_.first();
    }

    constexpr T2& second() noexcept
    {
        return data_.second();
    }

    // TODO

    // TODO: Make this conditionally explicit if one of the types is not *implicitly* default constructible.
    template<typename U1 = T1, typename U2 = T2, std::enable_if_t<
        detail_::is_default_constructible_v<U1, U2>,
        int> = 0>
    constexpr pair() noexcept(detail_::is_nothrow_default_constructible_v<T1, T2>)
    {
    }

    template<typename U1 = T1, typename U2 = T2, std::enable_if_t<
        (std::is_copy_constructible_v<U1> && std::is_copy_constructible_v<U2>) &&
        (std::is_convertible_v<const U1&, U1> && std::is_convertible_v<const U2&, U2>),
        int> = 0>
    constexpr pair(const T1& a, const T2& b)
        : data_(a, b)
    {
    }

    template<typename U1 = T1, typename U2 = T2, std::enable_if_t<(
        (std::is_copy_constructible_v<U1> && std::is_copy_constructible_v<U2>) &&
        (!std::is_convertible_v<const U1&, U1> || !std::is_convertible_v<const U2&, U2>)),
        int> = 0>
    explicit constexpr pair(const T1& a, const T2& b)
        : data_(a, b)
    {
    }
};
}

#endif
