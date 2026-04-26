/// @file rad_string.h
/// @author Graham Scott
/// @brief TODO
/// @date 2024-12-08
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_STRING_H_INCLUDED
#define RAD_STRING_H_INCLUDED

#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "rad_base.h"
#include "rad_allocator.h"
#include "rad_bit.h"

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
class basic_string;

template<
    typename CharType,
    class TraitsType = std::char_traits<CharType>>
class basic_cstring_view
{
    using inner_type_ = std::basic_string_view<CharType, TraitsType>;

    static constexpr CharType empty_string_data_[1] = { CharType{} };

    inner_type_ view_ = { empty_string_data_, 0 };

    void validate_range_(std::size_t index) const
    {
        if (index >= size())
        {
            throw std::out_of_range(
                "The given index was outside of the basic_cstring_view's range"
            );
        }
    }

    constexpr explicit basic_cstring_view(inner_type_ view) noexcept
        : view_(view)
    {
    }

public:
    using traits_type               = typename inner_type_::traits_type;
    using value_type                = typename inner_type_::value_type;
    using pointer                   = typename inner_type_::pointer;
    using const_pointer             = typename inner_type_::const_pointer;
    using reference                 = typename inner_type_::reference;
    using const_reference           = typename inner_type_::const_reference;
    using const_iterator            = typename inner_type_::const_iterator;
    using iterator                  = typename inner_type_::iterator;
    using const_reverse_iterator    = typename inner_type_::const_reverse_iterator;
    using reverse_iterator          = typename inner_type_::reverse_iterator;
    using size_type                 = typename inner_type_::size_type;
    using difference_type           = typename inner_type_::difference_type;

    static constexpr size_type npos = size_type(-1);

    constexpr size_type max_size() const noexcept
    {
        return view_.max_size();
    }

    constexpr const_pointer data() const noexcept
    {
        return view_.data();
    }

    constexpr const_pointer c_str() const noexcept
    {
        return view_.data();
    }

    constexpr size_type size() const noexcept
    {
        return view_.size();
    }

    constexpr size_type length() const noexcept
    {
        return view_.length();
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return view_.empty();
    }

    constexpr const_iterator begin() const noexcept
    {
        return view_.begin();
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return view_.cbegin();
    }

    constexpr const_iterator end() const noexcept
    {
        return view_.end();
    }

    constexpr const_iterator cend() const noexcept
    {
        return view_.cend();
    }

    constexpr const_iterator rbegin() const noexcept
    {
        return view_.rbegin();
    }

    constexpr const_iterator crbegin() const noexcept
    {
        return view_.crbegin();
    }

    constexpr const_iterator rend() const noexcept
    {
        return view_.rend();
    }

    constexpr const_iterator crend() const noexcept
    {
        return view_.crend();
    }

    constexpr const_reference front() const noexcept
    {
        return view_.front();
    }

    constexpr const_reference back() const noexcept
    {
        return view_.back();
    }

    constexpr void remove_prefix(size_type n)
    {
        view_.remove_prefix(n);
    }

    constexpr basic_cstring_view substr(size_type pos = 0) const
    {
        return basic_cstring_view{ view_.substr(pos) };
    }

    constexpr std::basic_string_view<CharType, TraitsType> substr(
        size_type pos,
        size_type count) const
    {
        return view_.substr(pos, count);
    }

    inline size_type copy(CharType* dest, size_type count, size_type pos = 0) const
    {
        return view_.copy(dest, count, pos);
    }

    constexpr int compare(std::basic_string_view<CharType, TraitsType> v) const noexcept
    {
        return view_.compare(v);
    }

    constexpr int compare(
        size_type pos1,
        size_type count1,
        std::basic_string_view<CharType, TraitsType> v) const
    {
        return view_.compare(pos1, count1, v);
    }

    constexpr int compare(
        size_type pos1,
        size_type count1,
        std::basic_string_view<CharType, TraitsType> v,
        size_type pos2,
        size_type count2) const
    {
        return view_.compare(pos1, count1, v, pos2, count2);
    }

    constexpr int compare(const CharType* s) const
    {
        return view_.compare(s);
    }

    constexpr int compare(
        size_type pos1,
        size_type count1,
        const CharType* s) const
    {
        return view_.compare(pos1, count1, s);
    }

    constexpr int compare(
        size_type pos1,
        size_type count1,
        const CharType* s,
        size_type count2) const
    {
        return view_.compare(pos1, count1, s, count2);
    }

    constexpr size_type find(
        std::basic_string_view<CharType, TraitsType> v,
        size_type pos = 0) const noexcept
    {
        return view_.find(v, pos);
    }

    constexpr size_type find(CharType ch, size_type pos = 0) const noexcept
    {
        return view_.find(ch, pos);
    }

    constexpr size_type find(const CharType* s, size_type pos, size_type count) const
    {
        return view_.find(s, pos, count);
    }

    constexpr size_type find(const CharType* s, size_type pos = 0) const
    {
        return view_.find(s, pos);
    }

    constexpr size_type rfind(
        std::basic_string_view<CharType, TraitsType> v,
        size_type pos = npos) const noexcept
    {
        return view_.rfind(v, pos);
    }

    constexpr size_type rfind(CharType ch, size_type pos = npos) const noexcept
    {
        return view_.rfind(ch, pos);
    }

    constexpr size_type rfind(const CharType* s, size_type pos, size_type count) const
    {
        return view_.rfind(s, pos, count);
    }

    constexpr size_type rfind(const CharType* s, size_type pos = npos) const
    {
        return view_.rfind(s, pos);
    }

    constexpr size_type find_first_of(
        std::basic_string_view<CharType, TraitsType> v,
        size_type pos = 0) const noexcept
    {
        return view_.find_first_of(v, pos);
    }

    constexpr size_type find_first_of(CharType ch, size_type pos = 0) const noexcept
    {
        return view_.find_first_of(ch, pos);
    }

    constexpr size_type find_first_of(const CharType* s, size_type pos, size_type count) const
    {
        return view_.find_first_of(s, pos, count);
    }

    constexpr size_type find_first_of(const CharType* s, size_type pos = 0) const
    {
        return view_.find_first_of(s, pos);
    }

    constexpr size_type find_first_not_of(
        std::basic_string_view<CharType, TraitsType> v,
        size_type pos = 0) const noexcept
    {
        return view_.find_first_not_of(v, pos);
    }

    constexpr size_type find_first_not_of(CharType ch, size_type pos = 0) const noexcept
    {
        return view_.find_first_not_of(ch, pos);
    }

    constexpr size_type find_first_not_of(const CharType* s, size_type pos, size_type count) const
    {
        return view_.find_first_not_of(s, pos, count);
    }

    constexpr size_type find_first_not_of(const CharType* s, size_type pos = 0) const
    {
        return view_.find_first_not_of(s, pos);
    }

    constexpr size_type find_last_of(
        std::basic_string_view<CharType, TraitsType> v,
        size_type pos = npos) const noexcept
    {
        return view_.find_last_of(v, pos);
    }

    constexpr size_type find_last_of(CharType ch, size_type pos = npos) const noexcept
    {
        return view_.find_last_of(ch, pos);
    }

    constexpr size_type find_last_of(const CharType* s, size_type pos, size_type count) const
    {
        return view_.find_last_of(s, pos, count);
    }

    constexpr size_type find_last_of(const CharType* s, size_type pos = npos) const
    {
        return view_.find_last_of(s, pos);
    }

    constexpr size_type find_last_not_of(
        std::basic_string_view<CharType, TraitsType> v,
        size_type pos = npos) const noexcept
    {
        return view_.find_last_not_of(v, pos);
    }

    constexpr size_type find_last_not_of(CharType ch, size_type pos = npos) const noexcept
    {
        return view_.find_last_not_of(ch, pos);
    }

    constexpr size_type find_last_not_of(const CharType* s, size_type pos, size_type count) const
    {
        return view_.find_last_not_of(s, pos, count);
    }

    constexpr size_type find_last_not_of(const CharType* s, size_type pos = npos) const
    {
        return view_.find_last_not_of(s, pos);
    }

    constexpr void swap(basic_cstring_view& other) noexcept
    {
        view_.swap(other.view_);
    }

    constexpr const_reference at(size_type pos) const
    {
        return view_.at(pos);
    }

    constexpr const std::basic_string_view<CharType, TraitsType>& view() const noexcept
    {
        return view_;
    }

    constexpr operator std::basic_string_view<CharType, TraitsType>() const noexcept
    {
        return view_;
    }

    constexpr const_reference operator[](size_type pos) const
        noexcept(!RAD_USE_STRICT_BOUNDS_CHECKING)
    {
    #if RAD_USE_STRICT_BOUNDS_CHECKING
        validate_range_(pos);
    #endif

        return view_[pos];
    }

    constexpr basic_cstring_view& operator=(
        const basic_cstring_view& other
    ) noexcept = default;

    constexpr basic_cstring_view() noexcept(
        std::is_nothrow_constructible_v<inner_type_, const CharType*, std::size_t>
    ) = default;

    constexpr basic_cstring_view(const basic_cstring_view& other) noexcept = default;

    constexpr basic_cstring_view(const CharType* str) noexcept(
        std::is_nothrow_constructible_v<inner_type_, const CharType*>)
        : view_(str)
    {
    }

    basic_cstring_view(std::nullptr_t) = delete;

    template<typename AllocatorType>
    inline basic_cstring_view(
        const std::basic_string<CharType, TraitsType, AllocatorType>& str) noexcept(
        std::is_nothrow_constructible_v<inner_type_, const CharType*, std::size_t>)
        : view_(str.c_str(), str.size())
    {
    }

    basic_cstring_view(
        const basic_string<CharType, TraitsType>& str
    ) noexcept(std::is_nothrow_constructible_v<inner_type_, const CharType*, std::size_t>);
};

using cstring_view = basic_cstring_view<char>;
using wcstring_view = basic_cstring_view<wchar_t>;
using u16cstring_view = basic_cstring_view<char16_t>;
using u32cstring_view = basic_cstring_view<char32_t>;

enum class string_copy_method
{
    /// @brief Only create a new buffer if the source string is owning.
    /// If the source string is non-owning, do not create a new buffer
    /// - just copy the existing pointer.
    soft_copy,

    /// @brief Always create a new buffer, regardless of whether
    /// the source string is owning or non-owning.
    hard_copy,
};

enum class string_copy_alloc_method
{
    /// @brief Create new buffer using the source string's allocator.
    src_allocator,

    /// @brief Create new buffer using the destination string's allocator.
    dst_allocator,
};

template<
    typename CharType,
    class TraitsType = std::char_traits<CharType>>
class basic_optional_string;

template<typename CharType, class TraitsType>
class basic_string
{
    friend basic_optional_string<CharType, TraitsType>;

    using size_type_ = std::size_t;

    static constexpr size_type_ no_ownership_flag_ = high_bit<size_type_>();

    static constexpr CharType empty_string_data_[1] = { CharType{} };

    rad::allocator*     allocator_ = &default_allocator;
    size_type_          count_ = no_ownership_flag_;
    const CharType*     data_ = empty_string_data_;

    static constexpr size_type_ validate_count_(size_type_ count)
    {
        if (count > max_size())
        {
            throw std::length_error(
                "The given count was greater than the maximum allowed size for strings"
            );
        }

        return count;
    }

    static size_type_ get_count_(const CharType* str)
    {
        assert(str != nullptr &&
            "The given string pointer should NEVER be null!"
        );

        return TraitsType::length(str);
    }

    static size_type_ get_valid_count_(const CharType* str)
    {
        return validate_count_(get_count_(str));
    }

    static size_type_ get_valid_count_or_no_own_flag_(const CharType* str)
    {
        const auto count = get_valid_count_(str);
        return (count == 0) ? no_ownership_flag_ : count;
    }

    static inline CharType* create_data_buffer_(
        rad::allocator& allocator,
        size_type_ count)
    {
        return allocator.create<CharType>(no_value_init, count + 1);
    }

    static CharType* create_data_copy_(
        rad::allocator& allocator,
        const CharType* str,
        size_type_ count)
    {
        assert((str != nullptr || count == 0) &&
            "The given string pointer must never be null "
            "unless count is also 0"
        );

        const auto data = create_data_buffer_(allocator, count);

        TraitsType::copy(data, str, count);
        data[count] = CharType{}; // Null-terminator

        return data;
    }

    static CharType* create_data_copy_(
        const basic_string& str,
        rad::allocator& allocator,
        string_copy_method copyMethod = string_copy_method::soft_copy)
    {
        return (copyMethod == string_copy_method::soft_copy && !str.owns_data()) ?
            const_cast<CharType*>(str.data_) :
            create_data_copy_(allocator, str.data_, str.count_);
    }

    static CharType* create_cstr_data_copy_(
        rad::allocator& allocator,
        const CharType* cstr,
        size_type_ count)
    {
        assert((cstr != nullptr || count == 0) &&
            "The given string pointer must never be null "
            "unless count is also 0"
        );

        const auto data = create_data_buffer_(allocator, count);
        TraitsType::copy(data, cstr, count + 1);
        return data;
    }

    void destroy_data_() noexcept
    {
        if (owns_data())
        {
            // NOTE: This if statement is only here to help optimize in debug builds.
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
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using pointer               = value_type*;
    using const_pointer         = const value_type*;
    using iterator              = pointer;
    using const_iterator        = const_pointer;
    // TODO: reverse_iterator
    // TODO: const_reverse_iterator

    static constexpr size_type max_size() noexcept
    {
        return ~no_ownership_flag_;
    }

    constexpr rad::allocator& allocator() const noexcept
    {
        return *allocator_;
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

    constexpr pointer data() noexcept
    {
        return const_cast<pointer>(data_);
    }

    constexpr const_pointer c_str() const noexcept
    {
        return data_;
    }

    constexpr basic_cstring_view<CharType, TraitsType> view() const noexcept
    {
        return *this;
    }

    pointer release() noexcept
    {
        const auto ptr = data_;

        count_ = no_ownership_flag_;
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

    basic_string& assign(
        const basic_string& str,
        string_copy_method copyMethod = string_copy_method::soft_copy,
        string_copy_alloc_method copyAllocMethod = string_copy_alloc_method::dst_allocator)
    {
        if (&str != this)
        {
            const auto newData = create_data_copy_(str,
                ((copyAllocMethod == string_copy_alloc_method::src_allocator) ?
                *str.allocator_ : *allocator_),
                copyMethod
            );

            destroy_data_();

            if (copyAllocMethod == string_copy_alloc_method::src_allocator)
            {
                allocator_ = str.allocator_;
            }

            count_ = (copyMethod == string_copy_method::hard_copy) ?
                str.size() : str.count_;

            data_ = newData;
        }
        
        return *this;
    }

    basic_string& assign(basic_string&& str) noexcept
    {
        if (&str != this)
        {
            destroy_data_();

            allocator_ = str.allocator_;
            count_ = str.count_;
            data_ = str.data_;

            str.count_ = no_ownership_flag_;
            str.data_ = empty_string_data_;
        }

        return *this;
    }

    basic_string& assign(const_pointer str, size_type count)
    {
        const_pointer newData;
        
        if (count == 0)
        {
            count = no_ownership_flag_;
            newData = empty_string_data_;
        }
        else
        {
            validate_count_(count);
            newData = create_data_copy_(*allocator_, str, count);
        }

        destroy_data_();

        count_ = count;
        data_ = newData;

        return *this;
    }

    inline basic_string& assign(const_pointer str)
    {
        return assign(str, get_count_(str));
    }

    constexpr operator std::basic_string_view<CharType, TraitsType>() const noexcept
    {
        return { data_, size() };
    }

    constexpr operator basic_optional_string<CharType, TraitsType>() const&;

    constexpr operator basic_optional_string<CharType, TraitsType>() &&;

    inline basic_string& operator=(const basic_string& other)
    {
        return assign(other);
    }

    inline basic_string& operator=(basic_string&& other) noexcept
    {
        return assign(std::move(other));
    }

    inline basic_string& operator=(const_pointer str)
    {
        return assign(str);
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
        , count_(no_ownership_flag_ | get_valid_count_(ptr))
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
        , count_(get_valid_count_(ptr))
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
        , count_(get_valid_count_or_no_own_flag_(str))
        , data_((count_ == no_ownership_flag_) ?
            empty_string_data_ :
            create_cstr_data_copy_(allocator, str, count_)
        )
    {
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
        , count_((count == 0) ? no_ownership_flag_ : validate_count_(count))
        , data_((count == 0) ? empty_string_data_ : create_data_copy_(allocator, str, count))
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

    basic_string(
        rad::allocator& allocator,
        const basic_string& other,
        string_copy_method copyMethod = string_copy_method::soft_copy)
        : allocator_(&allocator)
        , count_((copyMethod == string_copy_method::hard_copy) ?
            other.size() : other.count_)
        , data_(create_data_copy_(other, allocator, copyMethod))
    {
    }

    inline basic_string(
        const basic_string& other,
        string_copy_method copyMethod = string_copy_method::soft_copy)
        : basic_string(*other.allocator_, other, copyMethod)
    {
    }

    constexpr basic_string(basic_string&& other) noexcept
        : allocator_(other.allocator_)
        , count_(other.count_)
        , data_(other.data_)
    {
        other.count_ = no_ownership_flag_;
        other.data_ = empty_string_data_;
    }

    inline ~basic_string()
    {
        destroy_data_();
    }
};

template<typename CharType, typename TraitsType>
inline basic_cstring_view<CharType, TraitsType>::basic_cstring_view(
    const basic_string<CharType, TraitsType>& str) noexcept(
        std::is_nothrow_constructible_v<inner_type_, const CharType*, std::size_t>)
    : view_(str.c_str(), str.size())
{
}

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

template<typename CharType, class TraitsType>
inline bool operator!=(
    const basic_string<CharType, TraitsType>& a,
    const basic_string<CharType, TraitsType>& b)
{
    return (a.compare(b) != 0);
}

template<typename CharType, class TraitsType>
inline bool operator!=(
    const basic_string<CharType, TraitsType>& a,
    const CharType* b)
{
    return (a.compare(b) != 0);
}

template<typename CharType, class TraitsType>
inline bool operator!=(
    const CharType* a,
    const basic_string<CharType, TraitsType>& b)
{
    return (b.compare(a) != 0);
}

template<typename CharType, class TraitsType>
inline bool operator!=(
    const basic_string<CharType, TraitsType>& a,
    std::basic_string_view<CharType, TraitsType> b)
{
    return (a.compare(b) != 0);
}

template<typename CharType, class TraitsType>
inline bool operator!=(
    std::basic_string_view<CharType, TraitsType> a,
    const basic_string<CharType, TraitsType>& b)
{
    return (b.compare(a) != 0);
}

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

template<typename CharType, class TraitsType>
class basic_optional_string
{
    using inner_type_ = basic_string<CharType, TraitsType>;

    inner_type_ str_;

public:
    using traits_type           = TraitsType;
    using value_type            = CharType;
    using allocator_type        = rad::allocator;
    using size_type             = typename inner_type_::size_type;
    using difference_type       = std::ptrdiff_t;
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using pointer               = value_type*;
    using const_pointer         = const value_type*;
    using iterator              = pointer;
    using const_iterator        = const_pointer;
    // TODO: reverse_iterator
    // TODO: const_reverse_iterator

    static constexpr size_type max_size() noexcept
    {
        return inner_type_::max_size();
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

    constexpr pointer data() noexcept
    {
        return str_.data();
    }

    constexpr const_pointer c_str() const noexcept
    {
        return str_.c_str();
    }

    constexpr const basic_string<CharType, TraitsType>& value() const & noexcept
    {
        assert(has_value() &&
            "value() must never be called on optional strings "
            "which do not have a value! Please check using "
            "has_value() first."
        );

        return str_;
    }

    constexpr basic_string<CharType, TraitsType>& value() & noexcept
    {
        assert(has_value() &&
            "value() must never be called on optional strings "
            "which do not have a value! Please check using "
            "has_value() first."
        );

        return str_;
    }

    constexpr const basic_string<CharType, TraitsType>&& value() const && noexcept
    {
        assert(has_value() &&
            "value() must never be called on optional strings "
            "which do not have a value! Please check using "
            "has_value() first."
        );

        return std::move(str_);
    }

    constexpr basic_string<CharType, TraitsType>&& value() && noexcept
    {
        assert(has_value() &&
            "value() must never be called on optional strings "
            "which do not have a value! Please check using "
            "has_value() first."
        );

        return std::move(str_);
    }

    template<class U = basic_string<CharType, TraitsType>>
    constexpr basic_string<CharType, TraitsType> value_or(U&& defaultValue) const &
    {
        return (has_value()) ? value() :
            basic_string<CharType, TraitsType>(std::forward<U>(defaultValue));
    }

    template<class U = basic_string<CharType, TraitsType>>
    constexpr basic_string<CharType, TraitsType> value_or(U&& defaultValue) &&
    {
        return (has_value()) ? value() :
            basic_string<CharType, TraitsType>(std::forward<U>(defaultValue));
    }

    constexpr basic_cstring_view<CharType, TraitsType> view() const noexcept
    {
        return ((has_value()) ?
            str_.view() :
            basic_cstring_view<CharType, TraitsType>{}
        );
    }

    pointer release() noexcept
    {
        const auto ptr = str_.data_;

        str_.count_ = inner_type_::no_ownership_flag_;
        str_.data_ = nullptr;

        return ptr;
    }

    void clear() noexcept
    {
        str_.destroy_data_();
        release();
    }

    inline int compare(const basic_optional_string& other) const
    {
        if (other.has_value())
        {
            return (has_value()) ? str_.compare(other.str_) : -1;
        }
        else
        {
            return (has_value()) ? 1 : 0;
        }
    }

    inline int compare(std::basic_string_view<CharType, TraitsType> str) const
    {
        return str_.compare(str);
    }

    int compare(const_pointer str) const
    {
        if (str)
        {
            return (has_value()) ? str_.compare(str) : -1;
        }
        else
        {
            return (has_value()) ? 1 : 0;
        }
    }

    inline basic_optional_string& assign(
        const basic_string<CharType, TraitsType>& str,
        string_copy_method copyMethod = string_copy_method::soft_copy,
        string_copy_alloc_method copyAllocMethod = string_copy_alloc_method::dst_allocator)
    {
        str_.assign(str, copyMethod, copyAllocMethod);
        return *this;
    }

    basic_optional_string& assign(
        const basic_optional_string& str,
        string_copy_method copyMethod = string_copy_method::soft_copy,
        string_copy_alloc_method copyAllocMethod = string_copy_alloc_method::dst_allocator)
    {
        if (&str != this)
        {
            const auto newData = (!str.has_value()) ? nullptr :
                inner_type_::create_data_copy_(str.str_,
                    ((copyAllocMethod == string_copy_alloc_method::src_allocator) ?
                    str.allocator() : allocator()),
                    copyMethod
                );

            str_.destroy_data_();

            if (copyAllocMethod == string_copy_alloc_method::src_allocator)
            {
                str_.allocator_ = str.str_.allocator_;
            }

            str_.count = (copyMethod == string_copy_method::soft_copy || !str.has_value()) ?
                str.str_.count : str.size();

            str_.data_ = newData;
        }
        
        return *this;
    }

    basic_optional_string& assign(basic_string<CharType, TraitsType>&& str) noexcept
    {
        str_.assign(std::move(str));
        return *this;
    }

    basic_optional_string& assign(basic_optional_string&& str) noexcept
    {
        if (&str != this)
        {
            str_.destroy_data_();

            str_.allocator_ = str.str_.allocator_;
            str_.count_ = str.str_.count_;
            str_.data_ = str.str_.data_;

            str.str_.count_ = inner_type_::no_ownership_flag_;
            str.str_.data_ = nullptr;
        }

        return *this;
    }

    basic_optional_string& assign(const_pointer str, size_type count)
    {
        // NOTE: The usecase where str == nullptr and count > 0 is allowed.
        // In that case, the given count will just be ignored.

        const_pointer newData;
        
        if (!str)
        {
            count = inner_type_::no_ownership_flag_;
            newData = nullptr;
        }
        else if (count == 0)
        {
            count = inner_type_::no_ownership_flag_;
            newData = inner_type_::empty_string_data_;
        }
        else
        {
            inner_type_::validate_count_(count);
            newData = inner_type_::create_data_copy_(*str_.allocator_, str, count);
        }

        str_.destroy_data_();

        str_.count_ = count;
        str_.data_ = newData;

        return *this;
    }

    basic_optional_string& assign(const_pointer str)
    {
        // NOTE: We don't have to check if str == nullptr or not
        // because assign with nullptr and a count > 0 is allowed.
        return assign(str, inner_type_::get_count_(str));
    }

    constexpr explicit operator bool() const noexcept
    {
        return str_.data_ != nullptr;
    }

    inline basic_optional_string& operator=(const basic_string<CharType, TraitsType>& other)
    {
        return assign(other);
    }

    inline basic_optional_string& operator=(const basic_optional_string& other)
    {
        return assign(other);
    }

    inline basic_optional_string& operator=(basic_string<CharType, TraitsType>&& other) noexcept
    {
        return assign(std::move(other));
    }

    inline basic_optional_string& operator=(basic_optional_string&& other) noexcept
    {
        return assign(std::move(other));
    }

    inline basic_optional_string& operator=(const_pointer str)
    {
        return assign(str);
    }

    constexpr basic_optional_string() noexcept
        : str_(&default_allocator, inner_type_::no_ownership_flag_, nullptr)
    {
    }

    constexpr explicit basic_optional_string(rad::allocator& allocator) noexcept
        : str_(&allocator, inner_type_::no_ownership_flag_, nullptr)
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
        : str_(&allocator, (!ptr) ? inner_type_::no_ownership_flag_ :
            inner_type_::get_valid_count_(ptr),
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
        : str_(&allocator, (!ptr) ? inner_type_::no_ownership_flag_ :
            (inner_type_::no_ownership_flag_ | inner_type_::get_valid_count_(ptr)),
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
        : basic_optional_string(allocator)
    {
        if (str)
        {
            const auto count = inner_type_::get_valid_count_(str);
            if (count == 0)
            {
                str_.data_ = inner_type_::empty_string_data_;
            }
            else
            {
                str_.count_ = count;
                str_.data_ = inner_type_::create_cstr_data_copy_(allocator, str, count);
            }
        }
    }

    inline basic_optional_string(const_pointer str)
        : basic_optional_string(default_allocator, str)
    {
    }

    basic_optional_string(
        rad::allocator& allocator,
        const_pointer str,
        size_type count)
        : str_(&allocator,
            ((!str || count == 0) ?
                inner_type_::no_ownership_flag_ :
                inner_type_::validate_count_(count)
            ),
            ((!str) ? nullptr :
                ((count == 0) ? inner_type_::empty_string_data_ :
                inner_type_::create_data_copy_(allocator, str, count))
            )
        )
    {
    }

    inline basic_optional_string(const_pointer str, size_type count)
        : basic_optional_string(default_allocator, str, count)
    {
    }

    inline basic_optional_string(
        rad::allocator& allocator,
        std::basic_string_view<CharType, TraitsType> str)
        : str_(allocator, str.data(), str.size())
    {
    }

    inline basic_optional_string(
        std::basic_string_view<CharType, TraitsType> str)
        : str_(str.data(), str.size())
    {
    }

    inline basic_optional_string(
        rad::allocator& allocator,
        const basic_string<CharType, TraitsType>& other,
        string_copy_method copyMethod = string_copy_method::soft_copy)
        : str_(allocator, other, copyMethod)
    {
    }

    inline basic_optional_string(
        const basic_string<CharType, TraitsType>& other,
        string_copy_method copyMethod = string_copy_method::soft_copy)
        : str_(other, copyMethod)
    {
    }

    basic_optional_string(
        rad::allocator& allocator,
        const basic_optional_string& other,
        string_copy_method copyMethod = string_copy_method::soft_copy)
        : str_(&allocator,
            ((copyMethod == string_copy_method::soft_copy || !other.has_value()) ?
                other.str_.count_ :
                other.size()
            ),
            ((!other.has_value()) ?
                nullptr :
                inner_type_::create_data_copy_(other.str_, allocator, copyMethod)
            )
        )
    {
    }

    inline basic_optional_string(
        const basic_optional_string& other,
        string_copy_method copyMethod = string_copy_method::soft_copy)
        : basic_optional_string(other.allocator(), other, copyMethod)
    {
    }

    constexpr basic_optional_string(basic_string<CharType, TraitsType>&& other) noexcept
        : str_(std::move(other))
    {
    }

    constexpr basic_optional_string(basic_optional_string&& other) noexcept
        : str_(other.str_.allocator_, other.str_.count_, other.str_.data_)
    {
        other.str_.count_ = inner_type_::no_ownership_flag_;
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

template<typename CharType, class TraitsType>
inline bool operator==(
    const basic_optional_string<CharType, TraitsType>& a,
    const basic_optional_string<CharType, TraitsType>& b)
{
    return (a.compare(b) == 0);
}

template<typename CharType, class TraitsType>
inline bool operator==(
    const basic_optional_string<CharType, TraitsType>& a,
    const CharType* b)
{
    return (a.compare(b) == 0);
}

template<typename CharType, class TraitsType>
inline bool operator==(
    const CharType* a,
    const basic_optional_string<CharType, TraitsType>& b)
{
    return (b.compare(a) == 0);
}

template<typename CharType, class TraitsType>
inline bool operator==(
    const basic_optional_string<CharType, TraitsType>& a,
    std::basic_string_view<CharType, TraitsType> b)
{
    return (a.compare(b) == 0);
}

template<typename CharType, class TraitsType>
inline bool operator==(
    std::basic_string_view<CharType, TraitsType> a,
    const basic_optional_string<CharType, TraitsType>& b)
{
    return (b.compare(a) == 0);
}

template<typename CharType, class TraitsType>
inline bool operator!=(
    const basic_optional_string<CharType, TraitsType>& a,
    const basic_optional_string<CharType, TraitsType>& b)
{
    return (a.compare(b) != 0);
}

template<typename CharType, class TraitsType>
inline bool operator!=(
    const basic_optional_string<CharType, TraitsType>& a,
    const CharType* b)
{
    return (a.compare(b) != 0);
}

template<typename CharType, class TraitsType>
inline bool operator!=(
    const CharType* a,
    const basic_optional_string<CharType, TraitsType>& b)
{
    return (b.compare(a) != 0);
}

template<typename CharType, class TraitsType>
inline bool operator!=(
    const basic_optional_string<CharType, TraitsType>& a,
    std::basic_string_view<CharType, TraitsType> b)
{
    return (a.compare(b) != 0);
}

template<typename CharType, class TraitsType>
inline bool operator!=(
    std::basic_string_view<CharType, TraitsType> a,
    const basic_optional_string<CharType, TraitsType>& b)
{
    return (b.compare(a) != 0);
}
}

// Hash specializations

namespace std
{
template<typename CharType>
struct hash<rad::basic_cstring_view<CharType>>
{
    std::size_t operator()(const rad::basic_cstring_view<CharType>& csv) const noexcept
    {
        const auto sv = csv.view();
        return std::hash<std::remove_const_t<decltype(sv)>>{}(sv);
    }
};

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
#endif

#ifdef ANKERL_UNORDERED_DENSE_H
#ifndef RAD_STRING_SPEC_FOR_ANKERL_UNORDERED_DENSE
#define RAD_STRING_SPEC_FOR_ANKERL_UNORDERED_DENSE
namespace ankerl::unordered_dense
{
template<typename CharType>
struct hash<rad::basic_cstring_view<CharType>>
{
    using is_avalanching = void;

    std::uint64_t operator()(const rad::basic_cstring_view<CharType>& sv) const noexcept
    {
        return detail::wyhash::hash(sv.data(), sizeof(CharType) * sv.size());
    }
};

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

