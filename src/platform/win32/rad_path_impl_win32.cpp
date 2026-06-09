/// @file rad_path_impl_win32.cpp
/// @author Graham Scott
/// @brief Windows implementation of rad_path.h
/// @date 2024-08-22
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_path.h"
#include "rad_wide_path_impl_win32.h"

namespace rad::path
{
bool try_get_stats(entry_stats& entryStats, const char* path)
{
    const win32_wide_path_ widePath(path);
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;

    if (!GetFileAttributesExW(widePath, GetFileExInfoStandard, &fileInfo))
    {
        return false;
    }

    entryStats.size = (
        (static_cast<unsigned long long>(fileInfo.nFileSizeHigh) << 32) |
        fileInfo.nFileSizeLow
    );

    if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
    {
        auto handle = CreateFileW(
            widePath,
            0,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            nullptr
        );

        if (handle == INVALID_HANDLE_VALUE)
        {
    return false;
        }

        FILE_ATTRIBUTE_TAG_INFO tagInfo;
        const BOOL r = GetFileInformationByHandleEx(
            handle,
            FileAttributeTagInfo,
            &tagInfo,
            sizeof(tagInfo)
        );

        CloseHandle(handle);

        if (!r)
        {
            return false;
        }
        else if (tagInfo.ReparseTag == IO_REPARSE_TAG_SYMLINK)
        {
            entryStats.type = entry_type::symlink;
            return true;
        }
    }

    if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        entryStats.type = entry_type::directory;
    }
    else if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ||
        !(fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DEVICE))
    {
        entryStats.type = entry_type::regular_file;
    }
    else
    {
        entryStats.type = entry_type::other;
    }

    return true;
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
    const win32_wide_path_ widePath(path);
    return (GetFileAttributesW(widePath) != INVALID_FILE_ATTRIBUTES);
}

std::string canonicalize(const char* path)
{
    // TODO
    return path;
}
}
