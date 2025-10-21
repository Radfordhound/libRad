/// @file rad_string.h
/// @author Graham Scott
/// @brief TODO
/// @date 2024-12-08
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_STRING_H_INCLUDED
#define RAD_STRING_H_INCLUDED

#include "rad_default_allocator.h"
#include "rad_bit.h"
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <cstddef>
#include <cassert>

namespace rad
{
// TODO: Move this type to another header!
struct no_ownership_t final
{
    constexpr explicit no_ownership_t() noexcept = default;
};

[[maybe_unused]] constexpr no_ownership_t no_ownership{};

// TODO: Move this type to another header!
struct take_ownership_t final
{
    constexpr explicit take_ownership_t() noexcept = default;
};

[[maybe_unused]] constexpr take_ownership_t take_ownership{};

template<
    typename CharType,
    class TraitsType = std::char_traits<CharType>>
class basic_optional_string;

template<
    typename CharType,
    class TraitsType = std::char_traits<CharType>>
class basic_string
{
    friend basic_optional_string;

    using size_type_ = std::size_t;

    static constexpr size_type_ no_ownership_flag_ = high_bit<size_type_>();

    static constexpr CharType empty_string_data_[1] = { CharType{} };

    // !has_value():    data_ == nullptr
    // owns_data():     !(count_ & HIGH_BIT)
    // passing basic_string into const basic_optional_string&:  ALLOWED
    // passing basic_string into basic_optional_string&:        NOT ALLOWED
    // passing basic_optional_string into basic_string&:        NOT ALLOWED

    rad::allocator*     allocator_ = &default_allocator;
    size_type_          count_ = (no_ownership_flag_ | size_type_{1});
    const CharType*     data_ = empty_string_data_;

    static constexpr void validate_count_(size_type_ count)
    {
        if (count > max_size())
        {
            throw std::length_error(
                "The given count was greater than the maximum allowed size for strings"
            );
        }
    }

    static size_type_ get_count_(const CharType* str)
    {
        assert(str != nullptr &&
            "The given string pointer should NEVER be null!"
        );

        return TraitsType::length(str);
    }

    static size_type_ get_and_validate_count_(const CharType* str)
    {
        const auto count = get_count_(str);
        validate_count_(count);
        return count;
    }

    static CharType* create_data_buffer_(
        rad::allocator& allocator,
        size_type_ count)
    {
        validate_count_(count);
        return allocator.create<CharType>(no_value_init, count + 1);
    }

    static CharType* create_data_copy_(
        rad::allocator& allocator,
        const CharType* str,
        size_type_ count)
    {
        assert(str != nullptr || !count &&
            "The given string pointer should never be null "
            "unless count is also 0"
        );

        const auto data = create_data_buffer_(allocator, count);

        TraitsType::copy(data, str, count);
        data[count] = CharType{}; // Null-terminator

        return data;
    }

    static CharType* create_data_copy_(const basic_string& other)
    {
        /*
        if (!other.has_data())
        {
            return nullptr;
        }
        */

        // TODO: Is not copying if other is non-owning actually the behavior we want?
        // It's important for empty strings at least.

        return (!other.owns_data()) ?
            const_cast<CharType*>(other.data_) :
            create_data_copy_(*other.allocator_, other.data_, other.count_);
    }

    void destroy_data_() noexcept
    {
        if (owns_data())
        {
            if constexpr (std::is_trivially_destructible_v<CharType>)
            {
                allocator_->free(const_cast<CharType*>(data_));
            }
            else
            {
                allocator_->destroy(const_cast<CharType*>(data_), size());
            }
        }
    }

    constexpr basic_string(
        rad::allocator* allocator,
        size_type_ count,
        const CharType* data) noexcept
        : allocator_(allocator)
        , count_(count)
        , data_(data)
    {
    }

public:
    using traits_type           = TraitsType;
    using value_type            = CharType;
    using allocator_type        = rad::allocator;
    using size_type             = size_type_;
    using difference_type       = std::ptrdiff_t;
    using reference             = const value_type&;
    using const_reference       = const value_type&;
    using pointer               = const value_type*;
    using const_pointer         = const value_type*;
    using iterator              = pointer;
    using const_iterator        = const_pointer;
    // TODO: reverse_iterator
    // TODO: const_reverse_iterator

    static constexpr size_type max_size() noexcept
    {
        return no_ownership_flag_ - 1;
    }

    constexpr rad::allocator& allocator() const noexcept
    {
        return allocator_;
    }

    constexpr bool owns_data() const noexcept
    {
        return !(count_ & no_ownership_flag_);
    }

    constexpr size_type size() const noexcept
    {
        return count_ & ~no_ownership_flag_;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return !size();
    }

    constexpr const_pointer data() const noexcept
    {
        return data_;
    }

    constexpr const_pointer c_str() const noexcept
    {
        return data_;
    }

    constexpr std::basic_string_view<CharType, TraitsType> view() const noexcept
    {
        return { data_, size() };
    }

    pointer release() noexcept
    {
        const auto ptr = data_;

        count_ = (no_ownership_flag_ | size_type_{1});
        data_ = empty_string_data_;

        return ptr;
    }

    void clear() noexcept
    {
        destroy_data_();
        release();
    }

    int compare(const basic_string& other) const
    {
        return view().compare(other.view());
    }

    int compare(std::basic_string_view<CharType, TraitsType> str) const
    {
        return view().compare(str);
    }

    int compare(const_pointer str) const
    {
        return view().compare(str);
    }

    bool starts_with(CharType ch) const noexcept
    {
        return (
            size() != 0 &&
            TraitsType::eq(*data_, ch)
        );
    }

    bool starts_with(std::basic_string_view<CharType, TraitsType> str) const noexcept
    {
        return (
            size() >= str.size() &&
            TraitsType::compare(data_, str.data(), str.size()) == 0
        );
    }

    constexpr operator std::basic_string_view<CharType, TraitsType>() const noexcept
    {
        return { data_, size() };
    }

    constexpr operator basic_optional_string<CharType, TraitsType>() const&;

    constexpr operator basic_optional_string<CharType, TraitsType>() &&;

    basic_string& operator=(const basic_string& other)
    {
        if (&other != this)
        {
            const auto newData = create_data_copy_(other);

            destroy_data_();

            allocator_ = other.allocator_;
            count_ = other.count_;
            data_ = newData;
        }
        
        return *this;
    }

    basic_string& operator=(basic_string&& other) noexcept
    {
        if (&other != this)
        {
            destroy_data_();

            allocator_ = other.allocator_;
            count_ = other.count_;
            data_ = other.data_;

            other.count_ = (no_ownership_flag_ | size_type_{1});
            other.data_ = empty_string_data_;
        }

        return *this;
    }

    constexpr basic_string() noexcept = default;

    constexpr explicit basic_string(rad::allocator& allocator) noexcept
        : allocator_(&allocator)
    {
    }

    basic_string(std::nullptr_t) noexcept = delete;

    constexpr basic_string(
        no_ownership_t,
        rad::allocator& allocator,
        const_pointer ptr)
        : allocator_(&allocator)
        , count_(get_and_validate_count_(ptr) | no_ownership_flag_)
        , data_(ptr)
    {
    }

    constexpr basic_string(no_ownership_t, const_pointer ptr)
        : basic_string(no_ownership, default_allocator, ptr)
    {
    }

    constexpr basic_string(
        take_ownership_t,
        rad::allocator& allocator,
        pointer ptr)
        : allocator_(&allocator)
        , count_(get_and_validate_count_(ptr))
        , data_(ptr)
    {
    }

    constexpr basic_string(take_ownership_t, pointer ptr)
        : basic_string(take_ownership, default_allocator, ptr)
    {
    }

    basic_string(
        rad::allocator& allocator,
        const_pointer str)
        : allocator_(&allocator)
        , count_(get_count_(str))
        , data_(create_data_buffer_(allocator, count_))
    {
        TraitsType::copy(const_cast<CharType*>(data_), str, count_ + 1);
    }

    inline basic_string(const_pointer str)
        : basic_string(default_allocator, str)
    {
    }

    basic_string(
        rad::allocator& allocator,
        const_pointer str,
        size_type count)
        : allocator_(&allocator)
        , count_(count)
        , data_(create_data_copy_(allocator, str, count))
    {
    }

    inline basic_string(const_pointer str, size_type count)
        : basic_string(default_allocator, str, count)
    {
    }

    inline basic_string(
        rad::allocator& allocator,
        std::basic_string_view<CharType, TraitsType> str)
        : basic_string(allocator, str.data(), str.size())
    {
    }

    inline basic_string(
        std::basic_string_view<CharType, TraitsType> str)
        : basic_string(str.data(), str.size())
    {
    }

    inline basic_string(
        rad::allocator& allocator,
        const std::basic_string<CharType, TraitsType>& str)
        : basic_string(allocator, str.data(), str.size())
    {
    }

    inline basic_string(
        const std::basic_string<CharType, TraitsType>& str)
        : basic_string(str.data(), str.size())
    {
    }

    basic_string(const basic_string& other)
        : allocator_(other.allocator_)
        , count_(other.count_)
        , data_(create_data_copy_(other))
    {
    }

    constexpr basic_string(basic_string&& other) noexcept
        : allocator_(other.allocator_)
        , count_(other.count_)
        , data_(other.data_)
    {
        other.count_ = (no_ownership_flag_ | size_type_{1});
        other.data_ = empty_string_data_;
    }

    inline ~basic_string()
    {
        destroy_data_();
    }
};

template<typename CharType, class TraitsType>
inline bool operator==(
    const basic_string<CharType, TraitsType>& a,
    const basic_string<CharType, TraitsType>& b)
{
    return (a.compare(b) == 0);
}

template<typename CharType, class TraitsType>
inline bool operator==(
    const basic_string<CharType, TraitsType>& a,
    const CharType* b)
{
    return (a.compare(b) == 0);
}

template<typename CharType, class TraitsType>
inline bool operator==(
    const CharType* a,
    const basic_string<CharType, TraitsType>& b)
{
    return (b.compare(a) == 0);
}

template<typename CharType, class TraitsType>
inline bool operator==(
    const basic_string<CharType, TraitsType>& a,
    std::basic_string_view<CharType, TraitsType> b)
{
    return (a.compare(b) == 0);
}

template<typename CharType, class TraitsType>
inline bool operator==(
    std::basic_string_view<CharType, TraitsType> a,
    const basic_string<CharType, TraitsType>& b)
{
    return (b.compare(a) == 0);
}

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

template<
    typename CharType,
    class TraitsType>
class basic_optional_string
{
    using inner_type_ = basic_string<CharType, TraitsType>;

    inner_type_ str_;

    constexpr void validate_has_value_() const
    {
        if (!has_value())
        {
            throw std::runtime_error(
                "Attempted to access the value of an optional "
                "string which has no value"
            );
        }
    }

public:
    using traits_type           = TraitsType;
    using value_type            = CharType;
    using allocator_type        = rad::allocator;
    using size_type             = typename inner_type_::size_type;
    using difference_type       = std::ptrdiff_t;
    using reference             = const value_type&;
    using const_reference       = const value_type&;
    using pointer               = const value_type*;
    using const_pointer         = const value_type*;
    using iterator              = pointer;
    using const_iterator        = const_pointer;
    // TODO: reverse_iterator
    // TODO: const_reverse_iterator

    static constexpr size_type max_size() noexcept
    {
        return str_.max_size();
    }

    constexpr rad::allocator& allocator() const noexcept
    {
        return str_.allocator();
    }

    constexpr bool has_value() const noexcept
    {
        return str_.data_ != nullptr;
    }

    constexpr bool owns_data() const noexcept
    {
        return str_.owns_data();
    }

    constexpr size_type size() const noexcept
    {
        return str_.size();
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return str_.empty();
    }

    constexpr const_pointer data() const noexcept
    {
        return str_.data();
    }

    constexpr const_pointer c_str() const noexcept
    {
        return str_.c_str();
    }

    constexpr basic_string<CharType, TraitsType>& value() &
    {
        validate_has_value_();
        return str_;
    }

    constexpr const basic_string<CharType, TraitsType>& value() const &
    {
        validate_has_value_();
        return str_;
    }

    constexpr const basic_string<CharType, TraitsType>&& value() const &&
    {
        validate_has_value_();
        return std::move(str_);
    }

    constexpr basic_string<CharType, TraitsType>&& value() &&
    {
        validate_has_value_();
        return std::move(str_);
    }

    template<class U = basic_string<CharType, TraitsType>>
    constexpr basic_string<CharType, TraitsType> value_or(U&& defaultValue) const &
    {
        return (has_value()) ? **this : static_cast<
            basic_string<CharType, TraitsType>>(std::forward<U>(defaultValue));
    }

    template<class U = basic_string<CharType, TraitsType>>
    constexpr basic_string<CharType, TraitsType> value_or(U&& defaultValue) &&
    {
        return (has_value()) ? std::move(**this) : static_cast<
            basic_string<CharType, TraitsType>>(std::forward<U>(defaultValue));
    }

    constexpr std::basic_string_view<CharType, TraitsType> view() const noexcept
    {
        return ((has_value()) ?
            str_.view() :
            std::basic_string_view<CharType, TraitsType>{}
        );
    }

    void clear() noexcept
    {
        str_.destroy_data_();

        str_.count_ = 0;
        str_.data_ = nullptr;
    }

    constexpr const basic_string<CharType, TraitsType>& operator*() const & noexcept
    {
        return str_;
    }

    constexpr basic_string<CharType, TraitsType>& operator*() & noexcept
    {
        return str_;
    }

    constexpr const basic_string<CharType, TraitsType>&& operator*() const && noexcept
    {
        return std::move(str_);
    }

    constexpr basic_string<CharType, TraitsType>&& operator*() && noexcept
    {
        return std::move(str_);
    }

    constexpr explicit operator bool() const noexcept
    {
        return str_.data_ != nullptr;
    }

    basic_optional_string& operator=(const basic_optional_string& other)
    {
        if (&other != this)
        {
            const auto newData = (other.str_.data_) ?
                inner_type_::create_data_copy_(other.str_) :
                nullptr;

            str_.destroy_data_();

            str_.allocator_ = other.str_.allocator_;
            str_.count_ = other.str_.count_;
            str_.data_ = newData;
        }
        
        return *this;
    }

    basic_optional_string& operator=(basic_optional_string&& other) noexcept
    {
        if (&other != this)
        {
            str_.destroy_data_();

            str_.allocator_ = other.str_.allocator_;
            str_.count_ = other.str_.count_;
            str_.data_ = other.str_.data_;

            other.str_.count_ = 0;
            other.str_.data_ = nullptr;
        }

        return *this;
    }

    constexpr basic_optional_string() noexcept
        : str_(&default_allocator, 0, nullptr)
    {
    }

    constexpr explicit basic_optional_string(rad::allocator& allocator) noexcept
        : str_(&allocator, 0, nullptr)
    {
    }

    constexpr basic_optional_string(std::nullptr_t) noexcept
        : basic_optional_string()
    {
    }

    constexpr basic_optional_string(
        take_ownership_t,
        rad::allocator& allocator,
        pointer ptr)
        : str_(&allocator, (ptr) ?
            inner_type_::get_and_validate_count_(ptr) : 0,
            ptr)
    {
    }

    constexpr basic_optional_string(take_ownership_t, pointer ptr)
        : basic_optional_string(take_ownership, default_allocator, ptr)
    {
    }

    constexpr basic_optional_string(
        no_ownership_t,
        rad::allocator& allocator,
        const_pointer ptr)
        : str_(&allocator, (ptr) ?
            (inner_type_::get_and_validate_count_(ptr) |
            inner_type_::no_ownership_flag_) : 0,
            ptr)
    {
    }

    constexpr basic_optional_string(no_ownership_t, const_pointer ptr)
        : basic_optional_string(no_ownership, default_allocator, ptr)
    {
    }

    basic_optional_string(
        rad::allocator& allocator,
        const_pointer str)
        : str_(&allocator, 0, nullptr)
    {
        if (str_.data_)
        {
            str_.count_ = inner_type_::get_count_(str);
            str_.data_ = inner_type_::create_data_buffer_(allocator, str_.count_);

            TraitsType::copy(const_cast<CharType*>(str_.data_), str, str_.count_ + 1);
        }
    }

    inline explicit basic_optional_string(const_pointer str)
        : basic_optional_string(default_allocator, str)
    {
    }

    basic_optional_string(
        rad::allocator& allocator,
        const_pointer str,
        size_type count)
        : str_(&allocator, count, (str) ?
            inner_type_::create_data_copy_(allocator, str, count) :
            nullptr)
    {
    }

    inline basic_optional_string(const_pointer str, size_type count)
        : basic_optional_string(default_allocator, str, count)
    {
    }

    basic_optional_string(const basic_optional_string& other)
        : str_(other.str_.allocator_, other.str_.count_, (other.str_.data_) ?
            inner_type_::create_data_copy_(other.str_) :
            nullptr)
    {
    }

    constexpr basic_optional_string(basic_optional_string&& other) noexcept
        : str_(other.str_.allocator_, other.str_.count_, other.str_.data_)
    {
        other.str_.count_ = 0;
        other.str_.data_ = nullptr;
    }

    ~basic_optional_string() = default;
};

using optional_string = basic_optional_string<char>;
using optional_wstring = basic_optional_string<wchar_t>;
using optional_u16string = basic_optional_string<char16_t>;
using optional_u32string = basic_optional_string<char32_t>;

template<typename CharType, class TraitsType>
constexpr basic_string<CharType, TraitsType>::
    operator basic_optional_string<CharType, TraitsType>() const&
{
    // TODO: Pass count into optional string also so we don't have to recompute it
    return basic_optional_string<CharType, TraitsType>(
        no_ownership,
        *allocator_,
        data_
    );
}

template<typename CharType, class TraitsType>
constexpr basic_string<CharType, TraitsType>::
    operator basic_optional_string<CharType, TraitsType>() &&
{
    // TODO: Pass count into optional string also so we don't have to recompute it
    basic_optional_string<CharType, TraitsType> result(
        take_ownership,
        *allocator_,
        data_
    );
    
    release();
    
    return result;
}
}

// Hash specializations

namespace std
{
template<typename CharType>
struct hash<rad::basic_string<CharType>>
{
    std::size_t operator()(const rad::basic_string<CharType>& str) const noexcept
    {
        const auto sv = str.view();
        return std::hash<std::remove_const_t<decltype(sv)>>{}(sv);
    }
};

template<typename CharType>
struct hash<rad::basic_optional_string<CharType>>
{
    std::size_t operator()(const rad::basic_optional_string<CharType>& str) const noexcept
    {
        const auto sv = str.view();
        return std::hash<std::remove_const_t<decltype(sv)>>{}(sv);
    }
};
}

#ifdef ANKERL_UNORDERED_DENSE_H
namespace ankerl::unordered_dense
{
template<typename CharType>
struct hash<rad::basic_string<CharType>>
{
    using is_avalanching = void;

    std::uint64_t operator()(const rad::basic_string<CharType>& str) const noexcept
    {
        return detail::wyhash::hash(str.data(), sizeof(CharType) * str.size());
    }
};

template<typename CharType>
struct hash<rad::basic_optional_string<CharType>>
{
    using is_avalanching = void;

    std::uint64_t operator()(const rad::basic_optional_string<CharType>& str) const noexcept
    {
        return detail::wyhash::hash(str.data(), sizeof(CharType) * str.size());
    }
};
}
#endif

#endif
