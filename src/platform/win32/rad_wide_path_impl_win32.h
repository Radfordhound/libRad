/// @file rad_wide_path_impl_win32.h
/// @author Graham Scott
/// @brief Windows wide path helpers.
/// @date 2024-11-13
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_stack_or_heap_array.h"

namespace rad::path
{
class win32_wide_path_
{
    using buffer_t_ = stack_or_heap_array<WCHAR, MAX_PATH>;

    buffer_t_       buf_;

    static int convert_(LPCCH src, LPWSTR dst, int dstBufLen);

public:
    inline std::size_t size() const noexcept
    {
        return buf_.size() - 1;
    }

    inline const WCHAR* data() const noexcept
    {
        return buf_.data();
    }

    inline WCHAR* data() noexcept
    {
        return buf_.data();
    }

    inline operator const WCHAR*() const noexcept
    {
        return data();
    }

    win32_wide_path_(const char* path);
};
}
