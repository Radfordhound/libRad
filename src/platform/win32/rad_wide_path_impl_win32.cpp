/// @file rad_wide_path_impl_win32.cpp
/// @author Graham Scott
/// @brief Implementation of Windows wide path helpers.
/// @date 2024-11-13
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_wide_path_impl_win32.h"
#include <pathcch.h>

namespace rad::path
{
int win32_wide_path_::convert_(LPCCH src, LPWSTR dst, int dstBufLen)
{
    const auto r = MultiByteToWideChar(
        CP_UTF8,
        0,
        src,
        -1,
        dst,
        dstBufLen
    );

    if (!r) // TODO: Mark unlikely
    {
        throw std::runtime_error("Failed to convert UTF-8 path to UTF-16");
    }

    return r;
}

win32_wide_path_::win32_wide_path_(const char* path)
    : buf_(no_value_init, convert_(path, nullptr, 0))
{
    // Convert UTF-8 to UTF-16.
    convert_(path, buf_.data(), buf_.size());

    // Canonicalize paths greater than MAX_PATH.
    if (buf_.size() >= MAX_PATH) // TODO: Mark unlikely.
    {
        // TODO
        throw std::runtime_error(
            "Windows paths greater than MAX_PATH are not currently supported"
        );
    }
}
}
