/**
 * @file rad_pair.h
 * @author Graham Scott
 * @brief Header file providing rad::pair; a class similar to
 * std::pair, but with additional features/optimizations.
 * @version 0.1
 * @date 2023-04-04
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 */
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
    bool HasT1 = !std::is_empty_v<T1> && !std::is_final_v<T1>,
    bool HasT2 = !std::is_empty_v<T1> && !std::is_final_v<T1>>
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
        is_nothrow_default_constructible_v<T1, T2>) {}
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
        is_nothrow_default_constructible_v<T1, T2>) {}
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
        is_nothrow_default_constructible_v<T1, T2>) {}
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
        is_nothrow_default_constructible_v<T1, T2>) {}
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

    template<std::enable_if_t<detail_::is_default_constructible_v<T1, T2>, int> = 0>
    constexpr pair() noexcept(detail_::is_nothrow_default_constructible_v<T1, T2>) {}

    //template<std::enable_if_t<detail::is_copy_constructible_v<T1, T2>, int> = 0>
    //constexpr explicit()
};
} // rad

#endif
