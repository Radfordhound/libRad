/**
 * @file rad_object_utils.h
 * @author Graham Scott
 * @brief Header file providing utilities for managing objects (classes,
 * structs, primitive data types, etc.).
 * @version 0.1
 * @date 2023-04-03
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 */
#ifndef RAD_OBJECT_UTILS_H_INCLUDED
#define RAD_OBJECT_UTILS_H_INCLUDED

#include <type_traits>
#include <iterator>
#include <utility>

namespace rad
{
namespace detail_
{
    /**
     * @brief Return an rvalue reference to x if To can be nothrow
     * constructed from From&& or if To can be constructed from const From&.
     * Otherwise, returns a constant lvalue reference to x.
     * 
     * Usage: `::new (&a) TypeOfA(move_if_noexcept_construct_<TypeOfA>(b));`
     * 
     * @tparam To The type to check against to see if it can be nothrow-constructed from x.
     * @tparam From The type of x.
     * @param x The value to obtain an rvalue or lvalue reference to.
     * @return An rvalue reference to x if To can be nothrow constructed
     * from From&& or if To can be constructed from const From&. Otherwise, a
     * constant lvalue reference to x.
     */
    template<typename To, typename From>
    [[nodiscard]] constexpr std::conditional_t<
        !std::is_nothrow_constructible_v<To, std::add_rvalue_reference_t<From>> &&
        std::is_constructible_v<To, std::add_lvalue_reference_t<std::add_const_t<From>>>,
        const From&, From&&>
        move_if_noexcept_construct_(From& x) noexcept
    {
        return std::move(x);
    }

    /**
     * @brief Return an rvalue reference to x if From&& is nothrow
     * assignable to To& or if const From& is not assignable to To&.
     * Otherwise, returns a constant lvalue reference to x.
     * 
     * Usage: `a = move_if_noexcept_assign_<TypeOfA>(b);`
     * 
     * @tparam To The type to check against to see if it can be nothrow assigned from x.
     * @tparam From The type of x.
     * @param x The value to obtain an rvalue or lvalue reference to.
     * @return An rvalue reference to x if From&& is nothrow assignable to
     * To& or if const From& is not assignable to To&. Otherwise, a constant
     * lvalue reference to x.
     */
    template<typename To, typename From>
    [[nodiscard]] constexpr std::conditional_t<
        !std::is_nothrow_assignable_v<
            std::add_lvalue_reference_t<To>,
            std::add_rvalue_reference_t<From>> &&

        std::is_assignable_v<
            std::add_lvalue_reference_t<To>,
            std::add_lvalue_reference_t<std::add_const_t<From>>>,

        const From&, From&&>
        move_if_noexcept_assign_(From& x) noexcept
    {
        return std::move(x);
    }

    template<typename InputIt, typename OutputIt = InputIt>
    struct is_nothrow_uninitialized_movable_ : std::bool_constant<
        // Can we nothrow-construct OutputIt::value_type from InputIt::value_type&& ?
        // (nothrow move-construct)
        std::is_nothrow_constructible_v<
            typename std::iterator_traits<OutputIt>::value_type,
            std::add_rvalue_reference_t<
                typename std::iterator_traits<InputIt>::value_type>> ||

        // Can we nothrow-construct OutputIt::value_type from const InputIt::value_type& ?
        // (nothrow copy-construct)
        std::is_nothrow_constructible_v<
            typename std::iterator_traits<OutputIt>::value_type,
            std::add_lvalue_reference_t<std::add_const_t<
                typename std::iterator_traits<InputIt>::value_type>>>> {};

    template<typename InputIt, typename OutputIt = InputIt>
    inline constexpr bool is_nothrow_uninitialized_movable_v_ =
        is_nothrow_uninitialized_movable_<InputIt, OutputIt>::value;

    template<typename InputIt, typename OutputIt = InputIt>
    struct is_nothrow_movable_ : std::bool_constant<
        // Can we nothrow-assign InputIt::value_type&& to OutputIt::value_type& ?
        // (nothrow move-assign)
        std::is_nothrow_assignable_v<
            std::add_lvalue_reference_t<
                typename std::iterator_traits<OutputIt>::value_type>,
            std::add_rvalue_reference_t<
                typename std::iterator_traits<InputIt>::value_type>> ||

        // Or can we nothrow-assign const InputIt::value_type& to OutputIt::value_type& ?
        // (nothrow copy-assign)
        std::is_nothrow_assignable_v<
            std::add_lvalue_reference_t<
                typename std::iterator_traits<OutputIt>::value_type>,
            std::add_lvalue_reference_t<std::add_const_t<
                typename std::iterator_traits<InputIt>::value_type>>>> {};

    template<typename InputIt, typename OutputIt = InputIt>
    inline constexpr bool is_nothrow_movable_v_ =
        is_nothrow_movable_<InputIt, OutputIt>::value;

    template<typename InputIt, typename OutputIt = InputIt>
    struct is_nothrow_iterable_ : std::bool_constant<
        noexcept(++std::declval<InputIt&>()) &&                         // nothrow ++input
        noexcept(++std::declval<OutputIt&>()) &&                        // nothrow ++output
        noexcept(*std::declval<InputIt&>()) &&                          // nothrow *input
        noexcept(*std::declval<OutputIt&>()) &&                         // nothrow *output
        noexcept(std::declval<InputIt&>() != std::declval<InputIt&>())  // nothrow input != input
        > {};

    template<typename InputIt, typename OutputIt = InputIt>
    inline constexpr bool is_nothrow_iterable_v_ =
        is_nothrow_iterable_<InputIt, OutputIt>::value;
}

template<typename T>
constexpr void destruct(T& obj) noexcept
{
    // Arrays
    if constexpr (std::is_array_v<T>)
    {
        for (auto& v : obj)
        {
            destruct(v);
        }
    }

    // Non-Arrays
    else
    {
        obj.~T();
    }
}

template<typename Iterator>
constexpr void destruct(Iterator begin, Iterator end) noexcept(
    std::is_trivially_destructible_v<
        typename std::iterator_traits<Iterator>::value_type> ||
    detail_::is_nothrow_iterable_v_<Iterator>)
{
    if constexpr (!std::is_trivially_destructible_v<
        typename std::iterator_traits<Iterator>::value_type>)
    {
        for (; begin != end; ++begin)
        {
            destruct(*begin);
        }
    }
}

template<typename Iterator, typename... Args>
auto uninitialized_direct_construct(Iterator begin, Iterator end, const Args&... args)
    noexcept(std::is_nothrow_constructible_v<
        typename std::iterator_traits<Iterator>::value_type,
        const Args&...>)
    -> std::enable_if_t<detail_::is_nothrow_iterable_v_<Iterator>>
{
    using namespace detail_;
    using T = typename std::iterator_traits<Iterator>::value_type;

    // nothrow
    if constexpr (std::is_nothrow_constructible_v<
        typename std::iterator_traits<Iterator>::value_type,
        const Args&...>)
    {
        for (; begin != end; ++begin)
        {
            ::new (const_cast<void*>(static_cast<const volatile void*>(
                std::addressof(*begin)))) T(args...);
        }
    }

    // maybe-throw
    else
    {
        Iterator it(begin); // NOTE: Iterator copy-construction itself can throw.
        try
        {
            for (; it != end; ++it)
            {
                ::new (const_cast<void*>(static_cast<const volatile void*>(
                    std::addressof(*it)))) T(args...);
            }
        }
        catch (...)
        {
            destruct(begin, it);
            throw;
        }
    }
}

template<typename Iterator, typename NoThrowForwardIt>
auto uninitialized_move_strong(Iterator begin,
    Iterator end, NoThrowForwardIt dst)
    noexcept(detail_::is_nothrow_uninitialized_movable_v_<
        Iterator, NoThrowForwardIt>)
    -> std::enable_if_t<detail_::is_nothrow_iterable_v_<
        Iterator, NoThrowForwardIt>, NoThrowForwardIt>
{
    using namespace detail_;
    using T = typename std::iterator_traits<NoThrowForwardIt>::value_type;

    // nothrow
    if constexpr (is_nothrow_uninitialized_movable_v_<Iterator, NoThrowForwardIt>)
    {
        for (; begin != end; ++begin, (void)++dst)
        {
            ::new (const_cast<void*>(static_cast<const volatile void*>(
                std::addressof(*dst)))) T(move_if_noexcept_construct_<T>(*begin));
        }

        return dst;
    }

    // maybe-throw
    else
    {
        NoThrowForwardIt it(dst); // NOTE: Iterator copy-construction itself can throw.
        try
        {
            for (; begin != end; ++begin, (void)++it)
            {
                ::new (const_cast<void*>(static_cast<const volatile void*>(
                    std::addressof(*it)))) T(move_if_noexcept_construct_<T>(*begin));
            }

            return it;
        }
        catch (...)
        {
            destruct(dst, it);
            throw;
        }
    }
}

/**
 * @brief Moves the elements from the given input range to
 * the given output destination, with strong exception guarantee.
 * 
 * Like std::move from <algorithm>, except with a strong exception guarantee.
 * 
 * The input range will be move-assigned to the output, unless the input is not
 * nothrow-move-assignable to the output, in which case, the input will be copy-assigned
 * to the output instead, unless the input can ONLY be move-assigned AND is not
 * nothrow-move-assignable, in which case, the input must be move-assigned and the
 * strong exception guarantee will be waived.
 * 
 * If the input is nothrow-move-assignable or nothrow-copy-assignable to the output, and
 * the input and output iterators do not throw, this function is marked as noexcept, and
 * is guaranteed to never throw.
 * 
 * Otherwise, this function may throw, but guarantees that if this happens, the input will
 * ALWAYS be left in the same state as it was before this function was called, unless the
 * strong exception guarantee was waived as described above.
 * 
 * @tparam InputIt The type of iterator used to iterate over the given input range.
 * @tparam OutputIt The type of iterator used to iterate over the given output.
 * @param begin The first element in the input range.
 * @param end One past the last element in the input range.
 * @param dst The first element in the output range.
 * @return OutputIt One past the last element in the output range.
 */
template<typename InputIt, typename OutputIt>
OutputIt move_strong(InputIt begin, InputIt end, OutputIt dst) noexcept(
    detail_::is_nothrow_movable_v_<InputIt, OutputIt> &&
    detail_::is_nothrow_iterable_v_<InputIt, OutputIt>)
{
    using T = typename std::iterator_traits<OutputIt>::value_type;

    for (; begin != end; ++begin, (void)++dst)
    {
        // Try to nothrow move-assign. Fallback to copy-assign, unless
        // type is not copy-assignable, in which case, fallback again
        // to move-assign and waive strong exception guarantee.
        *dst = detail_::move_if_noexcept_assign_<T>(*begin);
    }

    return dst;
}
}

#endif
