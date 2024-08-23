/// @file rad_path_unix.cpp
/// @author Graham Scott
/// @brief Implementation of rad_path_unix.h
/// @date 2024-07-03
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_path_unix.h"

using namespace std::string_view_literals;

namespace rad::path
{
bool has_trailing_separator_unix(std::string_view path) noexcept
{
    return (!path.empty() && is_separator_unix(path.back()));
}

bool has_leading_separator_unix(std::string_view path) noexcept
{
    return (!path.empty() && is_separator_unix(path.front()));
}

static std::size_t get_trailing_separator_count_unix_(
    std::string_view path) noexcept
{
    std::size_t i = path.size();

    while (i > 0)
    {
        if (!is_separator_unix(path[i - 1]))
        {
            break;
        }

        --i;
    }

    return (path.size() - i);
}

std::string_view get_no_trailing_separator_path_unix(std::string_view path) noexcept
{
    path.remove_suffix(get_trailing_separator_count_unix_(path));
    return path;
}

static std::size_t get_file_name_index_unix_(std::string_view path) noexcept
{
    std::size_t i = path.size();

    while (i > 0)
    {
        --i;

        if (is_separator_unix(path[i]))
        {
            return (i + 1);
        }
    }
    
    return 0;
}

std::string_view get_name_unix(std::string_view path) noexcept
{
    path.remove_suffix(get_trailing_separator_count_unix_(path));
    path.remove_prefix(get_file_name_index_unix_(path));

    return path;
}

static std::size_t get_extensions_index_unix_(std::string_view path) noexcept
{
    std::size_t i = path.size();
    std::size_t extsIndex = i;

    while (i > 0)
    {
        --i;

        if (path[i] == '.')
        {
            extsIndex = i;
        }
        else if (is_separator_unix(path[i]))
        {
            break;
        }
    }
    
    return extsIndex;
}

std::string_view get_extensions_unix(std::string_view path) noexcept
{
    path.remove_suffix(get_trailing_separator_count_unix_(path));
    path.remove_prefix(get_extensions_index_unix_(path));

    return path;
}

std::string_view get_parent_unix(std::string_view path) noexcept
{
    path.remove_suffix(get_trailing_separator_count_unix_(path));
    return std::string_view(path.data(), get_file_name_index_unix_(path));
}

static std::size_t get_leading_separator_count_unix_(std::string_view str)
{
    const std::size_t len = str.size();
    
    for (std::size_t i = 0; i < len; ++i)
    {
        if (!is_separator_unix(str[i]))
        {
            return i;
        }
    }

    return len;
}

bool append_unix(std::string& path, std::string_view component)
{
    if (!component.empty())
    {
        const bool needsSep = !has_trailing_separator_unix(path);
        component.remove_prefix(get_leading_separator_count_unix_(component));

        const std::size_t appendLen = (needsSep + component.size());

        if (appendLen)
        {
            path.reserve(path.capacity() + appendLen);

            if (needsSep)
            {
                path += '/';
            }

            path += component;
            return true;
        }
    }

    return false;
}

std::string combine_unix(std::string_view path1, std::string_view path2)
{
    if (!path2.empty())
    {
        const bool needsSep = !has_trailing_separator_unix(path1);
        path2.remove_prefix(get_leading_separator_count_unix_(path2));

        const std::size_t appendLen = (needsSep + path2.size());

        if (appendLen)
        {
            std::string result;
            result.reserve(path1.size() + appendLen);

            result += path1;

            if (needsSep)
            {
                result += '/';
            }

            result += path2;
            return result;
        }
    }

    return std::string(path1);
}

bool remove_trailing_separators_unix(std::string& path)
{
    const auto trailingSepsCount = get_trailing_separator_count_unix_(path);
    path.erase(path.size() - trailingSepsCount);

    return (trailingSepsCount != 0);
}

bool remove_trailing_separators_unix(std::string_view& path)
{
    const auto trailingSepsCount = get_trailing_separator_count_unix_(path);
    path.remove_suffix(trailingSepsCount);

    return (trailingSepsCount != 0);
}

bool remove_name_unix(std::string& path)
{
    const auto noSepsPath = get_no_trailing_separator_path_unix(path);
    const auto fileNameIndex = get_file_name_index_unix_(noSepsPath);

    path.erase(fileNameIndex);

    return (fileNameIndex != 0);
}

bool remove_name_unix(std::string_view& path)
{
    const auto noSepsPath = get_no_trailing_separator_path_unix(path);
    const auto fileNameIndex = get_file_name_index_unix_(noSepsPath);

    path = std::string_view(path.data(), fileNameIndex);

    return (fileNameIndex != 0);
}

std::size_t component_iterator_unix::get_initial_component_length_(const char* path)
{
    // Handle root directory special case (e.g. the forward-slash in "/whatever").
    return (is_separator_unix(*path)) ?
        1 : get_current_component_length_(path);
}

const char* component_iterator_unix::skip_repeating_separators(const char* path)
{
    // NOTE: We do not need to check for the null-terminator
    // explicitly, since a null-terminator will cause
    // is_separator_unix to return false, thus ending the loop.
    while (is_separator_unix(*path))
    {
        ++path;
    }

    return path;
}

std::size_t component_iterator_unix::get_current_component_length_(const char* path)
{
    std::size_t len = 0;

    while (
        path[len] != '\0' &&
        !is_separator_unix(path[len]))
    {
        ++len;
    }
    
    return len;
}

component_iterator_unix& component_iterator_unix::operator++()
{
    path_ = skip_repeating_separators(path_ + curComponentLen_);
    curComponentLen_ = get_current_component_length_(path_);

    return *this;
}

component_iterator_unix component_iterator_unix::operator++(int)
{
    const auto it = *this;
    ++(*this);
    return it;
}

component_iterator_unix::component_iterator_unix(const char* path)
    : path_(path)
    , curComponentLen_(get_initial_component_length_(path_))
{
}

component_iterator_unix::component_iterator_unix(const std::string& path)
    : path_(path.c_str())
    , curComponentLen_(get_initial_component_length_(path_))
{
}

component_iterator_unix::component_iterator_unix(std::string_view path)
    : path_(path.data())
    , curComponentLen_(path.empty() ? 0 : get_initial_component_length_(path_))
{
}

component_iterator_unix get_begin_unix(const char* path)
{
    return component_iterator_unix(path);
}

component_iterator_unix get_begin_unix(const std::string& path)
{
    return component_iterator_unix(path);
}

component_iterator_unix get_begin_unix(std::string_view path)
{
    return component_iterator_unix(path);
}

component_iterator_unix get_end_unix(const char* path)
{
    return component_iterator_unix(path + std::strlen(path));
}

component_iterator_unix get_end_unix(const std::string& path)
{
    return component_iterator_unix(path.c_str() + path.size());
}

component_iterator_unix get_end_unix(std::string_view path)
{
    return ((path.empty()) ?
        component_iterator_unix() :
        component_iterator_unix(path.data() + path.size())
    );
}

component_iterators_unix::component_iterators_unix(
    component_iterator_unix begin,
    component_iterator_unix end) noexcept
    : begin_(begin)
    , end_(end)
{
}

component_iterators_unix components_unix(const char* path)
{
    return component_iterators_unix(
        get_begin_unix(path),
        get_end_unix(path)
    );
}

component_iterators_unix components_unix(const std::string& path)
{
    return component_iterators_unix(
        get_begin_unix(path),
        get_end_unix(path)
    );
}

component_iterators_unix components_unix(std::string_view path)
{
    return component_iterators_unix(
        get_begin_unix(path),
        get_end_unix(path)
    );
}
}
