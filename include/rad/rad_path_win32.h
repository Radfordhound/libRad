/// @file rad_path_win32.h
/// @author Graham Scott
/// @brief Header file providing path helper utilities for Windows paths.
/// @date 2024-07-14
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_PATH_WIN32_H_INCLUDED
#define RAD_PATH_WIN32_H_INCLUDED

#include "rad_base.h"
#include <string_view>
#include <string>
#include <cstddef>

namespace rad::path
{
constexpr bool is_separator_win32(char c) noexcept
{
    return (c == '/' || c == '\\');
}

RAD_API bool has_trailing_separator_win32(std::string_view path) noexcept;

inline bool has_leading_separator_win32(const char* path) noexcept
{
    // NOTE: This overload exists to take advantage of the fact that
    // C-strings are supposed to always end with a null-terminator, thus
    // we don't have to check its length first.
    return is_separator_win32(*path);
}

inline bool has_leading_separator_win32(const std::string& path) noexcept
{
    // NOTE: This overload exists to take advantage of the fact that
    // std::strings are guaranteed to end with a null-terminator, thus
    // we don't have to check if it's empty first.
    return is_separator_win32(*path.c_str());
}

RAD_API bool has_leading_separator_win32(std::string_view path) noexcept;

RAD_API std::string_view get_no_trailing_separator_path_win32(std::string_view path) noexcept;

RAD_API std::string_view get_name_win32(std::string_view path) noexcept;

RAD_API std::string_view get_extensions_win32(std::string_view path) noexcept;

RAD_API std::string_view get_parent_win32(std::string_view path) noexcept;

RAD_API bool append_win32(std::string& path, std::string_view component);

RAD_API std::string combine_win32(std::string_view path1, std::string_view path2);

RAD_API bool remove_trailing_separators_win32(std::string& path);

RAD_API bool remove_trailing_separators_win32(std::string_view& path);

RAD_API bool remove_name_win32(std::string& path);

RAD_API bool remove_name_win32(std::string_view& path);

class component_iterator_win32
{
    const char*     path_ = nullptr;
    std::size_t     curComponentLen_ = 0;

    static std::size_t get_initial_component_length_(const char* path);

    static const char* skip_repeating_separators(const char* path);

    static std::size_t get_current_component_length_(const char* path);

public:
    using value_type = std::string_view;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    RAD_API component_iterator_win32& operator++();

    RAD_API component_iterator_win32 operator++(int);

    inline value_type operator*() const noexcept
    {
        return { path_, curComponentLen_ };
    }

    inline bool operator==(const component_iterator_win32& other) const noexcept
    {
        return (path_ == other.path_);
    }

    inline bool operator!=(const component_iterator_win32& other) const noexcept
    {
        return (path_ != other.path_);
    }

    constexpr component_iterator_win32() noexcept = default;

    RAD_API explicit component_iterator_win32(const char* path);

    RAD_API explicit component_iterator_win32(const std::string& path);

    RAD_API explicit component_iterator_win32(std::string_view path);
};

RAD_API component_iterator_win32 get_begin_win32(const char* path);

RAD_API component_iterator_win32 get_begin_win32(const std::string& path);

RAD_API component_iterator_win32 get_begin_win32(std::string_view path);

RAD_API component_iterator_win32 get_end_win32(const char* path);

RAD_API component_iterator_win32 get_end_win32(const std::string& path);

RAD_API component_iterator_win32 get_end_win32(std::string_view path);

class component_iterators_win32
{
    component_iterator_win32 begin_;
    component_iterator_win32 end_;

public:
    inline component_iterator_win32 begin() const noexcept
    {
        return begin_;
    }

    inline component_iterator_win32 end() const noexcept
    {
        return end_;
    }

    constexpr component_iterators_win32() noexcept = default;

    RAD_API component_iterators_win32(
        component_iterator_win32 begin,
        component_iterator_win32 end
    ) noexcept;
};

RAD_API component_iterators_win32 components_win32(const char* path);

RAD_API component_iterators_win32 components_win32(const std::string& path);

RAD_API component_iterators_win32 components_win32(std::string_view path);
}

#endif
