/// @file rad_path_win32.cpp
/// @author Graham Scott
/// @brief Implementation of rad_path_win32.h
/// @date 2024-07-03
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_path_win32.h"

using namespace std::string_view_literals;

namespace rad::path
{
bool has_trailing_separator_win32(std::string_view path) noexcept
{
    return (!path.empty() && is_separator_win32(path.back()));
}

bool has_leading_separator_win32(std::string_view path) noexcept
{
    return (!path.empty() && is_separator_win32(path.front()));
}

static std::size_t get_trailing_separator_count_win32_(
    std::string_view path) noexcept
{
    std::size_t i = path.size();

    while (i > 0)
    {
        if (!is_separator_win32(path[i - 1]))
        {
            break;
        }

        --i;
    }

    return (path.size() - i);
}

static std::size_t get_file_name_index_win32_(std::string_view path) noexcept
{
    std::size_t i = path.size();

    while (i > 0)
    {
        --i;

        if (is_separator_win32(path[i]) ||
            path[i] == ':')
        {
            return (i + 1);
        }
    }
    
    return 0;
}

std::string_view get_no_trailing_separator_path_win32(std::string_view path) noexcept
{
    path.remove_suffix(get_trailing_separator_count_win32_(path));
    return path;
}

std::string_view get_name_win32(std::string_view path) noexcept
{
    path.remove_suffix(get_trailing_separator_count_win32_(path));

    if (!path.empty() &&
        path.back() != ':' &&   // e.g. "C:"
        path.back() != '?' &&   // e.g. "\\?"
        path != "\\\\."sv)      // e.g. "\\."
    {
        // OPTIMIZATION: We skip checking anything else in the path
        // since valid paths can't utilize colons or question mark
        // characters anywhere but in their prefix anyway.
        
        path.remove_prefix(get_file_name_index_win32_(path));
        return path;
    }

    return std::string_view();
}

static std::size_t get_extensions_index_win32_(std::string_view path) noexcept
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
        else if (is_separator_win32(path[i]) ||
            path[i] == ':')
        {
            break;
        }
    }
    
    return extsIndex;
}

std::string_view get_extensions_win32(std::string_view path) noexcept
{
    path.remove_suffix(get_trailing_separator_count_win32_(path));

    if (path != "\\\\."sv)  // e.g. "\\."
    {
        path.remove_prefix(get_extensions_index_win32_(path));
    }

    return path;
}

std::string_view get_parent_win32(std::string_view path) noexcept
{
    path.remove_suffix(get_trailing_separator_count_win32_(path));

    if (!path.empty() &&
        path.back() != ':' &&   // e.g. "C:"
        path.back() != '?' &&   // e.g. "\\?"
        path != "\\\\."sv)      // e.g. "\\."
    {
        // OPTIMIZATION: We skip checking anything else in the path
        // since valid paths can't utilize colons or question mark
        // characters anywhere but in their prefix anyway.
        
        path.remove_suffix(get_file_name_index_win32_(path));
    }

    return path;
}

static std::size_t get_leading_separator_count_win32_(std::string_view str)
{
    const std::size_t len = str.size();
    
    for (std::size_t i = 0; i < len; ++i)
    {
        if (!is_separator_win32(str[i]))
        {
            return i;
        }
    }

    return len;
}

bool append_win32(std::string& path, std::string_view component)
{
    if (!component.empty())
    {
        const bool needsSep = !has_trailing_separator_win32(path);
        component.remove_prefix(get_leading_separator_count_win32_(component));

        const std::size_t appendLen = (needsSep + component.size());

        if (appendLen)
        {
            path.reserve(path.capacity() + appendLen);

            if (needsSep)
            {
                path += '\\';
            }

            path += component;
            return true;
        }
    }

    return false;
}

std::string combine_win32(std::string_view path1, std::string_view path2)
{
    if (!path2.empty())
    {
        const bool needsSep = !has_trailing_separator_win32(path1);
        path2.remove_prefix(get_leading_separator_count_win32_(path2));

        const std::size_t appendLen = (needsSep + path2.size());

        if (appendLen)
        {
            std::string result;
            result.reserve(path1.size() + appendLen);

            result += path1;

            if (needsSep)
            {
                result += '\\';
            }

            result += path2;
            return result;
        }
    }

    return std::string(path1);
}

bool remove_trailing_separators_win32(std::string& path)
{
    const auto trailingSepsCount = get_trailing_separator_count_win32_(path);
    path.erase(path.size() - trailingSepsCount);

    return (trailingSepsCount != 0);
}

bool remove_trailing_separators_win32(std::string_view& path)
{
    const auto trailingSepsCount = get_trailing_separator_count_win32_(path);
    path.remove_suffix(trailingSepsCount);

    return (trailingSepsCount != 0);
}

bool remove_name_win32(std::string& path)
{
    const auto noSepsPath = get_no_trailing_separator_path_win32(path);

    if (!noSepsPath.empty() &&
        noSepsPath.back() != ':' &&   // e.g. "C:"
        noSepsPath.back() != '?' &&   // e.g. "\\?"
        noSepsPath != "\\\\."sv)      // e.g. "\\."
    {
        // OPTIMIZATION: We skip checking anything else in the path
        // since valid paths can't utilize colons or question mark
        // characters anywhere but in their prefix anyway.
        
        const auto fileNameIndex = get_file_name_index_win32_(noSepsPath);
        path.erase(fileNameIndex);

        return (fileNameIndex != 0);
    }

    return false;
}

bool remove_name_win32(std::string_view& path)
{
    const auto noSepsPath = get_no_trailing_separator_path_win32(path);

    if (!noSepsPath.empty() &&
        noSepsPath.back() != ':' &&   // e.g. "C:"
        noSepsPath.back() != '?' &&   // e.g. "\\?"
        noSepsPath != "\\\\."sv)      // e.g. "\\."
    {
        // OPTIMIZATION: We skip checking anything else in the path
        // since valid paths can't utilize colons or question mark
        // characters anywhere but in their prefix anyway.
        
        const auto fileNameIndex = get_file_name_index_win32_(noSepsPath);
        path = std::string_view(path.data(), fileNameIndex);

        return (fileNameIndex != 0);
    }

    return false;
}

std::size_t component_iterator_win32::get_initial_component_length_(const char* path)
{
    // Handle path prefixes that can only occur at the very beginning of a valid path.
    if (path[0] == '\\' && path[1] == '\\')
    {
        // Handle the following special prefixes: "\\?\" and "\\.\".
        if ((path[2] == '?' || path[2] == '.') &&
            path[3] == '\\')
        {
            return 4;
        }

        // Handle UNC path prefixes: "\\".
        else
        {
            return 2;
        }
    }
    
    return get_current_component_length_(path);
}

const char* component_iterator_win32::skip_repeating_separators(const char* path)
{
    // NOTE: We do not need to check for the null-terminator
    // explicitly, since a null-terminator will cause
    // is_separator_win32 to return false, thus ending the loop.
    while (is_separator_win32(*path))
    {
        ++path;
    }

    return path;
}

std::size_t component_iterator_win32::get_current_component_length_(const char* path)
{
    std::size_t len = 0;

    while (
        path[len] != '\0' &&
        !is_separator_win32(path[len]) &&

        // NOTE: We purposefully postfix-increment len *here*, so
        // that ':' characters ARE counted in the component length,
        // but also cause the while loop to end.
        path[len++] != ':')
    {
    }
    
    return len;
}

component_iterator_win32& component_iterator_win32::operator++()
{
    // Handle root directory special case (e.g. the backslash in "C:\whatever").
    if (curComponentLen_ > 0 &&
        path_[curComponentLen_ - 1] == ':' &&
        is_separator_win32(path_[curComponentLen_]))
    {
        path_ += curComponentLen_;
        curComponentLen_ = 1;
    }

    // Get the next component of the path.
    else
    {
        path_ = skip_repeating_separators(path_ + curComponentLen_);
        curComponentLen_ = get_current_component_length_(path_);
    }

    return *this;
}

component_iterator_win32 component_iterator_win32::operator++(int)
{
    const auto it = *this;
    ++(*this);
    return it;
}

component_iterator_win32::component_iterator_win32(const char* path)
    : path_(path)
    , curComponentLen_(get_initial_component_length_(path_))
{
}

component_iterator_win32::component_iterator_win32(const std::string& path)
    : path_(path.c_str())
    , curComponentLen_(get_initial_component_length_(path_))
{
}

component_iterator_win32::component_iterator_win32(std::string_view path)
    : path_(path.data())
    , curComponentLen_(path.empty() ? 0 : get_initial_component_length_(path_))
{
}

component_iterator_win32 get_begin_win32(const char* path)
{
    return component_iterator_win32(path);
}

component_iterator_win32 get_begin_win32(const std::string& path)
{
    return component_iterator_win32(path);
}

component_iterator_win32 get_begin_win32(std::string_view path)
{
    return component_iterator_win32(path);
}

component_iterator_win32 get_end_win32(const char* path)
{
    return component_iterator_win32(path + std::strlen(path));
}

component_iterator_win32 get_end_win32(const std::string& path)
{
    return component_iterator_win32(path.c_str() + path.size());
}

component_iterator_win32 get_end_win32(std::string_view path)
{
    return ((path.empty()) ?
        component_iterator_win32() :
        component_iterator_win32(path.data() + path.size())
    );
}

component_iterators_win32::component_iterators_win32(
    component_iterator_win32 begin,
    component_iterator_win32 end) noexcept
    : begin_(begin)
    , end_(end)
{
}

component_iterators_win32 components_win32(const char* path)
{
    return component_iterators_win32(
        get_begin_win32(path),
        get_end_win32(path)
    );
}

component_iterators_win32 components_win32(const std::string& path)
{
    return component_iterators_win32(
        get_begin_win32(path),
        get_end_win32(path)
    );
}

component_iterators_win32 components_win32(std::string_view path)
{
    return component_iterators_win32(
        get_begin_win32(path),
        get_end_win32(path)
    );
}
}
