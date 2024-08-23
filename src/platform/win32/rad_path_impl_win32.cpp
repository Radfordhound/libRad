/// @file rad_path_impl_win32.cpp
/// @author Graham Scott
/// @brief Windows implementation of rad_path.h
/// @date 2024-08-22
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_path.h"

namespace rad::path
{
bool try_get_stats(entry_stats& entryStats, const char* path)
{
    // TODO
    return false;
}

entry_stats get_stats(const char* path)
{
    entry_stats entryStats;

    if (!try_get_stats(entryStats, path))
    {
        throw std::system_error(GetLastError(), std::generic_category());
    }

    return entryStats;
}

bool exists(const char* path)
{
    // TODO
    return false;
}

std::string canonicalize(const char* path)
{
    // TODO
    return path;
}
}
