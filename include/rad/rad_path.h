/// @file rad_path.h
/// @author Graham Scott
/// @brief Header file providing path helper utilities for all supported platforms.
/// @date 2024-06-29
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_PATH_H_INCLUDED
#define RAD_PATH_H_INCLUDED

#include "rad_path_win32.h"
#include "rad_path_unix.h"
#include <string>
#include <string_view>

namespace rad::path
{
#ifndef RAD_PATH_IS_WIN32_TARGET
    #ifdef _WIN32
        #define RAD_PATH_IS_WIN32_TARGET 1
    #else
        #define RAD_PATH_IS_WIN32_TARGET 0
    #endif
#endif

/// @brief Represents the preferred path separator character on the target platform.
constexpr char separator =
#if RAD_PATH_IS_WIN32_TARGET == 1
    '\\';
#else
    '/';
#endif

constexpr bool is_separator(char c) noexcept
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return is_separator_win32(c);
#else
    return is_separator_unix(c);
#endif
}

inline bool has_trailing_separator(std::string_view path) noexcept
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return has_trailing_separator_win32(path);
#else
    return has_trailing_separator_unix(path);
#endif
}

inline bool has_leading_separator(const char* path) noexcept
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return has_leading_separator_win32(path);
#else
    return has_leading_separator_unix(path);
#endif
}

inline bool has_leading_separator(const std::string& path) noexcept
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return has_leading_separator_win32(path);
#else
    return has_leading_separator_unix(path);
#endif
}

inline bool has_leading_separator(std::string_view path) noexcept
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return has_leading_separator_win32(path);
#else
    return has_leading_separator_unix(path);
#endif
}

inline std::string_view get_no_trailing_separator_path(std::string_view path) noexcept
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return get_no_trailing_separator_path_win32(path);
#else
    return get_no_trailing_separator_path_unix(path);
#endif
}

inline std::string_view get_name(std::string_view path) noexcept
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return get_name_win32(path);
#else
    return get_name_unix(path);
#endif
}

inline std::string_view get_extensions(std::string_view path) noexcept
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return get_extensions_win32(path);
#else
    return get_extensions_unix(path);
#endif
}

inline std::string_view get_parent(std::string_view path) noexcept
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return get_parent_win32(path);
#else
    return get_parent_unix(path);
#endif
}

inline bool append(std::string& path, std::string_view component)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return append_win32(path, component);
#else
    return append_unix(path, component);
#endif
}

inline std::string combine(std::string_view path1, std::string_view path2)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return combine_win32(path1, path2);
#else
    return combine_unix(path1, path2);
#endif
}

inline bool remove_trailing_separators(std::string& path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return remove_trailing_separators_win32(path);
#else
    return remove_trailing_separators_unix(path);
#endif
}

inline bool remove_trailing_separators(std::string_view& path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return remove_trailing_separators_win32(path);
#else
    return remove_trailing_separators_unix(path);
#endif
}

inline bool remove_name(std::string& path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return remove_name_win32(path);
#else
    return remove_name_unix(path);
#endif
}

inline bool remove_name(std::string_view& path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return remove_name_win32(path);
#else
    return remove_name_unix(path);
#endif
}

using component_iterator =
#if RAD_PATH_IS_WIN32_TARGET == 1
    component_iterator_win32;
#else
    component_iterator_unix;
#endif

inline component_iterator get_begin(const char* path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return get_begin_win32(path);
#else
    return get_begin_unix(path);
#endif
}

inline component_iterator get_begin(const std::string& path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return get_begin_win32(path);
#else
    return get_begin_unix(path);
#endif
}

inline component_iterator get_begin(std::string_view path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return get_begin_win32(path);
#else
    return get_begin_unix(path);
#endif
}

inline component_iterator get_end(const char* path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return get_end_win32(path);
#else
    return get_end_unix(path);
#endif
}

inline component_iterator get_end(const std::string& path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return get_end_win32(path);
#else
    return get_end_unix(path);
#endif
}

inline component_iterator get_end(std::string_view path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return get_end_win32(path);
#else
    return get_end_unix(path);
#endif
}

using component_iterators =
#if RAD_PATH_IS_WIN32_TARGET == 1
    component_iterators_win32;
#else
    component_iterators_unix;
#endif

inline component_iterators components(const char* path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return components_win32(path);
#else
    return components_unix(path);
#endif
}

inline component_iterators components(const std::string& path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return components_win32(path);
#else
    return components_unix(path);
#endif
}

inline component_iterators components(std::string_view path)
{
#if RAD_PATH_IS_WIN32_TARGET == 1
    return components_win32(path);
#else
    return components_unix(path);
#endif
}

enum class entry_type
{
    other = 0,
    regular_file,
    directory,
    symlink,
};

struct entry_stats
{
    entry_type type;
    unsigned long long  size;

    constexpr bool is_other() const noexcept
    {
        return type == entry_type::other;
    }

    constexpr bool is_regular_file() const noexcept
    {
        return type == entry_type::regular_file;
    }

    constexpr bool is_directory() const noexcept
    {
        return type == entry_type::directory;
    }

    constexpr bool is_symlink() const noexcept
    {
        return type == entry_type::symlink;
    }
};

RAD_API bool try_get_stats(entry_stats& entryStats, const char* path);

inline bool try_get_stats(entry_stats& entryStats, const std::string& path)
{
    return try_get_stats(entryStats, path.c_str());
}

RAD_API entry_stats get_stats(const char* path);

inline entry_stats get_stats(const std::string& path)
{
    return get_stats(path.c_str());
}

RAD_API bool exists(const char* path);

inline bool exists(const std::string& path)
{
    return exists(path.c_str());
}

RAD_API std::string canonicalize(const char* path);

inline std::string canonicalize(const std::string& path)
{
    return canonicalize(path.c_str());
}
}

#endif
