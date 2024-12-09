/// @file rad_memory.h
/// @author Graham Scott
/// @brief Header file providing memory-management helper utilities.
/// @date 2023-04-07
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_MEMORY_H_INCLUDED
#define RAD_MEMORY_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <iterator>
#include <utility>
#include <memory>

namespace rad
{
namespace detail_
{
    template<typename InputIt, typename OutputIt = InputIt>
    struct is_nothrow_uninitialized_movable : std::bool_constant<
        // Can we nothrow-construct OutputIt::value_type from InputIt::value_type&& ?
        // (nothrow-move-construct)
        std::is_nothrow_constructible_v<
            typename std::iterator_traits<OutputIt>::value_type,
            std::add_rvalue_reference_t<
                typename std::iterator_traits<InputIt>::value_type>> ||

        // Can we nothrow-construct OutputIt::value_type from const InputIt::value_type& ?
        // (nothrow-copy-construct)
        std::is_nothrow_constructible_v<
            typename std::iterator_traits<OutputIt>::value_type,
            std::add_lvalue_reference_t<std::add_const_t<
                typename std::iterator_traits<InputIt>::value_type>>>
    > {};

    template<typename InputIt, typename OutputIt = InputIt>
    inline constexpr bool is_nothrow_uninitialized_movable_v =
        is_nothrow_uninitialized_movable<InputIt, OutputIt>::value;

    template<typename InputIt, typename OutputIt = InputIt>
    struct is_nothrow_movable : std::bool_constant<
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
                typename std::iterator_traits<InputIt>::value_type>>>
    > {};

    template<typename InputIt, typename OutputIt = InputIt>
    inline constexpr bool is_nothrow_movable_v =
        is_nothrow_movable<InputIt, OutputIt>::value;

    template<typename InputIt>
    struct is_nothrow_forward_iterable : std::bool_constant<
        noexcept(++std::declval<InputIt&>()) &&                         // nothrow ++input
        noexcept(*std::declval<InputIt&>())                             // nothrow *input
    > {};

    template<typename InputIt>
    inline constexpr bool is_nothrow_forward_iterable_v =
        is_nothrow_forward_iterable<InputIt>::value;

    template<typename InputIt, typename OutputIt = InputIt>
    struct is_nothrow_forward_loopable : std::bool_constant<
        is_nothrow_forward_iterable_v<InputIt> &&
        is_nothrow_forward_iterable_v<OutputIt> &&
        noexcept(std::declval<InputIt&>() != std::declval<InputIt&>())  // nothrow input != input
    > {};

    template<typename InputIt, typename OutputIt = InputIt>
    inline constexpr bool is_nothrow_forward_loopable_v =
        is_nothrow_forward_loopable<InputIt, OutputIt>::value;
}

struct no_default_init_t final
{
    constexpr explicit no_default_init_t() noexcept = default;
};

[[maybe_unused]] constexpr no_default_init_t no_default_init{};

struct no_value_init_t final
{
    constexpr explicit no_value_init_t() noexcept = default;
};

[[maybe_unused]] constexpr no_value_init_t no_value_init{};

constexpr bool is_aligned(std::uintptr_t address, std::size_t alignment) noexcept
{
    return ((address % alignment) == 0);
}

inline bool is_aligned(const void* ptr, std::size_t alignment) noexcept
{
    return is_aligned(reinterpret_cast<std::uintptr_t>(ptr), alignment);
}

/// @brief Returns an rvalue reference to x if `To` can be nothrow-constructed
/// from `From&&` or if `To` can not be constructed from `const From&`.
/// Otherwise, returns a constant lvalue reference to x.
/// 
/// @details Usage: `new (&a) TypeOfA(move_if_nothrow_construct<TypeOfA>(b));`
///
/// If `TypeOfA` can be nothrow-move-constructed from b, or if it can
/// not be copy-constructed from b, this example is equivalent to:
///
/// `new (&a) TypeOfA(std::move(b)); /* Move-construction */`
///
/// Otherwise, this example is equivalent to:
///
/// `new (&a) TypeOfA(b); /* Copy-construction */`
///
/// This allows you to write templated code which does move-construction
/// with a strong exception guarantee.
/// 
/// @tparam To The type to check against to see if it can be nothrow-constructed from x.
/// @tparam From The type of x.
/// @param x The value to obtain an rvalue or constant lvalue reference to.
///
/// @return An rvalue reference to x if `To` can be nothrow-constructed
/// from `From&&` or if `To` can not be constructed from `const From&`.
/// Otherwise, a constant lvalue reference to x.
template<typename To, typename From>
[[nodiscard]] constexpr auto move_if_nothrow_construct(From& x) noexcept
    -> std::conditional_t<
        (!std::is_nothrow_constructible_v<To, std::add_rvalue_reference_t<From>> &&
        std::is_constructible_v<To, std::add_lvalue_reference_t<std::add_const_t<From>>>),

        std::add_lvalue_reference_t<std::add_const_t<From>>, // const From&
        std::add_rvalue_reference_t<From>> // From&&
{
    return std::move(x);
}

/// @brief Returns an rvalue reference to x if `To` can not be constructed
/// from `const From&`. Otherwise, returns a constant lvalue reference to x.
/// 
/// @details Usage: `new (&a) TypeOfA(move_if_not_copy_constructible<TypeOfA>(b));`
///
/// If `TypeOfA` can not be copy-constructed from b, this example is equivalent to:
///
/// `new (&a) TypeOfA(std::move(b)); /* Move-construction */`
///
/// Otherwise, this example is equivalent to:
///
/// `new (&a) TypeOfA(b); /* Copy-construction */`
///
/// This allows you to write templated code which copy-constructs with a
/// strong exception guarantee, unless copy-construction is not possible,
/// in which case, it falls back to move-construction with a basic exception
/// guarantee.
/// 
/// @tparam To The type to check against to see if it can be nothrow-constructed from x.
/// @tparam From The type of x.
/// @param x The value to obtain an rvalue or constant lvalue reference to.
///
/// @return An rvalue reference to x if `To` can not be constructed
/// from `const From&`. Otherwise, a constant lvalue reference to x.
template<typename To, typename From>
[[nodiscard]] constexpr auto move_if_not_copy_constructible(From& x) noexcept
    -> std::conditional_t<
        std::is_constructible_v<To, std::add_lvalue_reference_t<std::add_const_t<From>>>,
        std::add_lvalue_reference_t<std::add_const_t<From>>, // const From&
        std::add_rvalue_reference_t<From>> // From&&
{
    return std::move(x);
}

/// @brief Returns an rvalue reference to x if `From&&` is nothrow-assignable
/// to `To&` or if `const From&` is not assignable to `To&`.
/// Otherwise, returns a constant lvalue reference to x.
/// 
/// @details Usage: `a = move_if_nothrow_assign<TypeOfA>(b);`
///
/// If `TypeOfA` can be nothrow-move-assigned from b, or if it can
/// not be copy-assigned from b, this example is equivalent to:
///
/// `a = std::move(b); /* Move-assignment */`
///
/// Otherwise, this example is equivalent to:
///
/// `a = b; /* Copy-assignment */`
///
/// This allows you to write templated code which does move-assignment
/// with a strong exception guarantee.
/// 
/// @tparam To The type to check against to see if it can be nothrow-assigned from x.
/// @tparam From The type of x.
/// @param x The value to obtain an rvalue or lvalue reference to.
///
/// @return An rvalue reference to x if `From&&` is nothrow-assignable to
/// `To&` or if `const From&` is not assignable to `To&`.
/// Otherwise, a constant lvalue reference to x.
template<typename To, typename From>
[[nodiscard]] constexpr auto move_if_nothrow_assign(From& x) noexcept
    -> std::conditional_t<
        (!std::is_nothrow_assignable_v<
            std::add_lvalue_reference_t<To>,
            std::add_rvalue_reference_t<From>> &&

        std::is_assignable_v<
            std::add_lvalue_reference_t<To>,
            std::add_lvalue_reference_t<std::add_const_t<From>>>),

        std::add_lvalue_reference_t<std::add_const_t<From>>, // const From&
        std::add_rvalue_reference_t<From>> // From&&
{
    return std::move(x);
}

/// @brief Returns an rvalue reference to x if `const From&` is not assignable
/// to `To&`. Otherwise, returns a constant lvalue reference to x.
/// 
/// @details Usage: `a = move_if_not_copy_assignable<TypeOfA>(b);`
///
/// If `TypeOfA` can not be copy-assigned from b, this example is equivalent to:
///
/// `a = std::move(b); /* Move-assignment */`
///
/// Otherwise, this example is equivalent to:
///
/// `a = b; /* Copy-assignment */`
///
/// This allows you to write templated code which copy-assigns with a
/// strong exception guarantee, unless copy-assignment is not possible,
/// in which case, it falls back to move-assignment with a basic exception
/// guarantee.
/// 
/// @tparam To The type to check against to see if it can be copy-assigned from x.
/// @tparam From The type of x.
/// @param x The value to obtain an rvalue or constant lvalue reference to.
///
/// @return An rvalue reference to x if `const From&` is not assignable to
/// `To&`. Otherwise, a constant lvalue reference to x.
template<typename To, typename From>
[[nodiscard]] constexpr auto move_if_not_copy_assignable(From& x) noexcept
    -> std::conditional_t<
        std::is_assignable_v<
            std::add_lvalue_reference_t<To>,
            std::add_lvalue_reference_t<std::add_const_t<From>>>,

        std::add_lvalue_reference_t<std::add_const_t<From>>, // const From&
        std::add_rvalue_reference_t<From>> // From&&
{
    return std::move(x);
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
constexpr void destruct(Iterator begin, Iterator end)
    noexcept(
        std::is_trivially_destructible_v<typename std::iterator_traits<Iterator>::value_type> ||
        detail_::is_nothrow_forward_loopable_v<Iterator>
    )
{
    using T = typename std::iterator_traits<Iterator>::value_type;

    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        for (; begin != end; ++begin)
        {
            destruct(*begin);
        }
    }
}

template<typename Iterator, typename... Args>
void uninitialized_direct_construct(Iterator begin, Iterator end, const Args&... args)
    noexcept(
        detail_::is_nothrow_forward_loopable_v<Iterator> &&
        std::is_nothrow_constructible_v<
            typename std::iterator_traits<Iterator>::value_type,
            const Args&...>
    )
{
    using namespace detail_;
    using T = typename std::iterator_traits<Iterator>::value_type;

    // nothrow
    if constexpr (
        detail_::is_nothrow_forward_loopable_v<Iterator> &&
        std::is_nothrow_constructible_v<T, const Args&...>)
    {
        for (; begin != end; ++begin)
        {
            ::new (const_cast<void*>(
                static_cast<const volatile void*>(std::addressof(*begin))
            )) T(args...);
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
                ::new (const_cast<void*>(
                    static_cast<const volatile void*>(std::addressof(*it))
                )) T(args...);
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
auto uninitialized_move_strong(Iterator begin, Iterator end, NoThrowForwardIt dst)
    noexcept(
        detail_::is_nothrow_uninitialized_movable_v<Iterator, NoThrowForwardIt> &&
        detail_::is_nothrow_forward_loopable_v<Iterator, NoThrowForwardIt>
    )
    -> std::enable_if_t<
        detail_::is_nothrow_forward_iterable_v<NoThrowForwardIt>,
        NoThrowForwardIt
    >
{
    using namespace detail_;
    using T = typename std::iterator_traits<NoThrowForwardIt>::value_type;

    // nothrow
    if constexpr (
        is_nothrow_uninitialized_movable_v<Iterator, NoThrowForwardIt> &&
        is_nothrow_forward_loopable_v<Iterator, NoThrowForwardIt>)
    {
        for (; begin != end; ++begin, (void)++dst)
        {
            ::new (const_cast<void*>(
                static_cast<const volatile void*>(std::addressof(*dst))
            )) T(move_if_nothrow_construct<T>(*begin));
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
                if constexpr (is_nothrow_forward_loopable_v<Iterator, NoThrowForwardIt>)
                {
                    ::new (const_cast<void*>(
                        static_cast<const volatile void*>(std::addressof(*it))
                    )) T(move_if_nothrow_construct<T>(*begin));
                }
                else
                {
                    ::new (const_cast<void*>(
                        static_cast<const volatile void*>(std::addressof(*it))
                    )) T(move_if_not_copy_constructible<T>(*begin));
                }
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

/// @brief Moves the elements from the given input range to
/// the given output destination, with strong exception guarantee.
/// 
/// Like `std::move` from `<algorithm>`, except with a strong exception guarantee.
/// 
/// @details The input range will be move-assigned to the output unless the input is not
/// nothrow-move-assignable to the output. In this case, the input will be copy-assigned
/// to the output instead.
///
/// If the input can not be copy-assigned or nothrow-move-assigned, then the input will
/// be move-assigned to the output, and the strong exception guarantee will be waived.
/// 
/// If the input is nothrow-move-assignable or nothrow-copy-assignable to the output, and
/// the input and output iterators do not throw, this function is marked as noexcept, and
/// is guaranteed to never throw.
/// 
/// Otherwise, this function may throw, but it guarantees that if this happens, the input will
/// ALWAYS be left in the same state as it was before this function was called, unless the
/// strong exception guarantee was waived as described above.
/// 
/// @tparam InputIt The type of iterator used to iterate over the given input range.
/// @tparam OutputIt The type of iterator used to iterate over the given output.
/// @param begin The first element in the input range.
/// @param end One past the last element in the input range.
/// @param dst The first element in the output range.
/// @return OutputIt One past the last element in the output range.
template<typename InputIt, typename OutputIt>
OutputIt move_strong(InputIt begin, InputIt end, OutputIt dst)
    noexcept(
        detail_::is_nothrow_movable_v<InputIt, OutputIt> &&
        detail_::is_nothrow_forward_loopable_v<InputIt, OutputIt>
    )
{
    using T = typename std::iterator_traits<OutputIt>::value_type;

    if constexpr (detail_::is_nothrow_forward_loopable_v<InputIt, OutputIt>)
    {
        for (; begin != end; ++begin, (void)++dst)
        {
            // Try to nothrow-move-assign. Fallback to copy-assign, unless
            // type is not copy-assignable, in which case, fallback again
            // to move-assign and waive strong exception guarantee.
            *dst = move_if_nothrow_assign<T>(*begin);
        }
    }
    else
    {
        for (; begin != end; ++begin, (void)++dst)
        {
            *dst = move_if_not_copy_assignable<T>(*begin);
        }
    }

    return dst;
}
}

#endif
