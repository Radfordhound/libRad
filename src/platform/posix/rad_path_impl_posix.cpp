/// @file rad_path_impl_posix.cpp
/// @author Graham Scott
/// @brief POSIX implementation of rad_path.h
/// @date 2024-07-14
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_path.h"
#include <memory>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

namespace rad::path
{
static entry_type get_entry_type_(mode_t mode) noexcept
{
    if (S_ISLNK(mode))
    {
        return entry_type::symlink;
    }
    else if (S_ISREG(mode))
    {
        return entry_type::regular_file;
    }
    else if (S_ISDIR(mode))
    {
        return entry_type::directory;
    }
    else
    {
        return entry_type::other;
    }
}

bool try_get_stats(entry_stats& entryStats, const char* path)
{
    struct stat s;
    if (lstat(path, &s) == -1)
    {
        return false;
    }

    entryStats.type = get_entry_type_(s.st_mode);
    entryStats.size = s.st_size;
    return true;
}

entry_stats get_stats(const char* path)
{
    entry_stats entryStats;

    if (!try_get_stats(entryStats, path))
    {
        throw std::system_error(errno, std::generic_category());
    }

    return entryStats;
}

bool exists(const char* path)
{
    return (access(path, F_OK) != -1);
}

std::string canonicalize(const char* path)
{
    struct posix_realpath_deleter_
    {
        void operator()(char* str) const
        {
            std::free(str);
        }
    };

    using posix_realpath_ptr_ = std::unique_ptr<char[], posix_realpath_deleter_>;

    static_assert(_POSIX_VERSION >= 200112,
        "canonicalize cannot be correctly implemented on old POSIX versions "
        "as the old version of realpath has a major design flaw which makes "
        "it impossible to correctly use it with long paths"
    );

    const posix_realpath_ptr_ canonicalizedPath(
        realpath(path, nullptr)
    );

    return std::string(canonicalizedPath.get());
}
}
