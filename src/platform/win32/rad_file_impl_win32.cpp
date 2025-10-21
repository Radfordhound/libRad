/// @file rad_file_impl_win32.cpp
/// @author Graham Scott
/// @brief TODO
/// @date 2024-11-13
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_file.h"
#include "rad_wide_path_impl_win32.h"
#include <climits>

namespace rad
{
static constexpr unsigned long guaranteed_caps_ = (
    stream_capabilities::CAPS1_CAN_SEEK |
    stream_capabilities::CAPS1_CAN_GET_SIZE
);

static DWORD get_win32_desired_access_(unsigned long flags)
{
    switch (flags & file_stream::OPEN_MASK_MODE)
    {
    case file_stream::OPEN_MODE_READ_ONLY:
        return GENERIC_READ;

    case file_stream::OPEN_MODE_WRITE_ONLY:
        return GENERIC_WRITE;

    case file_stream::OPEN_MODE_READ_WRITE:
        return GENERIC_READ | GENERIC_WRITE;

    default:
        throw std::logic_error("Invalid open_flags combination");
    }
}

static DWORD get_win32_share_mode_(unsigned long flags)
{
    if (flags & file_stream::OPEN_FLAG_SHARED)
    {
        switch (flags & file_stream::OPEN_MASK_MODE)
        {
        case file_stream::OPEN_MODE_READ_ONLY:
            return FILE_SHARE_READ;

        case file_stream::OPEN_MODE_WRITE_ONLY:
            return FILE_SHARE_WRITE;

        case file_stream::OPEN_MODE_READ_WRITE:
            return FILE_SHARE_READ | FILE_SHARE_WRITE;

        default:
            throw std::logic_error("Invalid open_flags combination");
        }
    }
    else
    {
        return 0;
    }
}

static DWORD get_win32_creation_disposition_(unsigned long flags)
{
    switch (flags & file_stream::OPEN_MASK_MODE)
    {
    case file_stream::OPEN_MODE_READ_ONLY:
        return OPEN_EXISTING;

    case file_stream::OPEN_MODE_WRITE_ONLY:
    case file_stream::OPEN_MODE_READ_WRITE:
        return (flags & file_stream::OPEN_FLAG_CREATE) ?
            CREATE_ALWAYS : OPEN_ALWAYS;

    default:
        throw std::logic_error("Invalid open_flags combination");
    }
}

static DWORD get_win32_flags_and_attributes_(unsigned long flags)
{
    switch (flags & file_stream::OPEN_MASK_HINT)
    {
    case file_stream::OPEN_HINT_NORMAL_ACCESS:
        return FILE_ATTRIBUTE_NORMAL;

    case file_stream::OPEN_HINT_SEQUENTIAL_ACCESS:
        return FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN;

    case file_stream::OPEN_HINT_RANDOM_ACCESS:
        return FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS;

    default:
        throw std::logic_error("Invalid open_flags combination");
    }
}

static stream_capabilities determine_capabilities_(unsigned long flags) noexcept
{
    unsigned long caps = guaranteed_caps_;

    switch (flags & file_stream::OPEN_MASK_MODE)
    {
    case file_stream::OPEN_MODE_READ_ONLY:
        caps |= stream_capabilities::CAPS1_CAN_READ;
        break;
    
    case file_stream::OPEN_MODE_WRITE_ONLY:
        caps |= stream_capabilities::CAPS1_CAN_WRITE;
        break;

    case file_stream::OPEN_MODE_READ_WRITE:
        caps |= stream_capabilities::CAPS1_CAN_READ | stream_capabilities::CAPS1_CAN_WRITE;
        break;
    }

    return caps;
}

static stream_capabilities determine_capabilities_(bool canRead, bool canWrite) noexcept
{
    unsigned long caps = guaranteed_caps_;

    if (canRead)
    {
        caps |= stream_capabilities::CAPS1_CAN_READ;
    }

    if (canWrite)
    {
        caps |= stream_capabilities::CAPS1_CAN_WRITE;
    }

    return caps;
}

std::uintmax_t file_stream::try_open_(const char* filePath, unsigned long flags)
{
    const path::win32_wide_path_ widePath(filePath);
    SECURITY_ATTRIBUTES securityAttributes = {};

    securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAttributes.bInheritHandle = TRUE;

    const auto fileHandle = CreateFileW(
        widePath,
        get_win32_desired_access_(flags),
        get_win32_share_mode_(flags),
        &securityAttributes,
        get_win32_creation_disposition_(flags),
        get_win32_flags_and_attributes_(flags),
        NULL
    );

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        const auto err = GetLastError();
        if (err != ERROR_FILE_NOT_FOUND)
        {
            throw std::system_error(err, std::generic_category());
        }
    }

    return reinterpret_cast<std::uintmax_t>(fileHandle);
}

bool file_stream::try_close_(std::uintmax_t handle) noexcept
{
    return CloseHandle(reinterpret_cast<HANDLE>(handle));
}

std::size_t file_stream::try_read_chunk_(void* buf, std::size_t size)
{
    const auto fileHandle = reinterpret_cast<HANDLE>(handle_);
    DWORD bytesRead;

    if (!ReadFile(
        fileHandle,
        buf,
        static_cast<DWORD>(size),
        &bytesRead,
        nullptr))
    {
        // TODO: Do we need to throw in some cases like with POSIX?
        return 0;
    }

    pos_ += bytesRead;
    return bytesRead;
}

std::size_t file_stream::try_write_chunk_(const void* buf, std::size_t size)
{
    const auto fileHandle = reinterpret_cast<HANDLE>(handle_);
    DWORD bytesWritten;

    if (!WriteFile(
        fileHandle,
        buf,
        static_cast<DWORD>(size),
        &bytesWritten,
        nullptr))
    {
        // TODO: Do we need to throw in some cases like with POSIX?
        return 0;
    }

    pos_ += bytesWritten;
    return bytesWritten;
}

unsigned long long file_stream::get_size() const
{
    const auto fileHandle = reinterpret_cast<HANDLE>(handle_);
    LARGE_INTEGER fileSize;

    if (!GetFileSizeEx(
        fileHandle,
        &fileSize))
    {
        throw std::system_error(GetLastError(), std::generic_category());
    }
    
    return fileSize.QuadPart;
}

void file_stream::flush()
{
    assert(caps_.can_write() &&
        "This stream does not have the capability to write"
    );

    const auto fileHandle = reinterpret_cast<HANDLE>(handle_);

    if (!FlushFileBuffers(fileHandle))
    {
        throw std::system_error(GetLastError(), std::generic_category());
    }
}

std::size_t file_stream::try_read(void* buf, std::size_t size)
{
    assert(caps_.can_read() &&
        "This stream does not have the capability to read"
    );

    const auto dstBegin = static_cast<unsigned char*>(buf);
    auto curDst = dstBegin;

    // Read in chunks.
    static constexpr std::size_t chunkSize = (4 * 1024 * 1024); // 4MiB

    while (size > chunkSize)
    {
        const auto bytesRead = try_read_chunk_(curDst, chunkSize);
        curDst += bytesRead;

        if (bytesRead < chunkSize)
        {
            return static_cast<std::size_t>(curDst - dstBegin);
        }

        size -= bytesRead;
    }

    // Read the final chunk (often this will be the only actual call to try_read_chunk_).
    curDst += try_read_chunk_(curDst, size);

    return static_cast<std::size_t>(curDst - dstBegin);
}

std::size_t file_stream::try_write(const void* buf, std::size_t size)
{
    assert(caps_.can_write() &&
        "This stream does not have the capability to write"
    );

    const auto dstBegin = static_cast<const unsigned char*>(buf);
    auto curDst = dstBegin;

    // Write in chunks.
    static constexpr std::size_t chunkSize = (4 * 1024 * 1024); // 4MiB

    while (size > chunkSize)
    {
        const auto bytesWritten = try_write_chunk_(curDst, chunkSize);
        curDst += bytesWritten;

        if (bytesWritten < chunkSize)
        {
            return static_cast<std::size_t>(curDst - dstBegin);
        }

        size -= bytesWritten;
    }

    // Write the final chunk (often this will be the only actual call to try_write_chunk_).
    curDst += try_write_chunk_(curDst, size);

    return static_cast<std::size_t>(curDst - dstBegin);
}

static constexpr DWORD get_win32_seek_mode_(file_stream::seek_mode mode) noexcept
{
    switch (mode)
    {
    case file_stream::seek_mode::current:
        return FILE_CURRENT;
    
    case file_stream::seek_mode::end:
        return FILE_END;

    default:
        return FILE_BEGIN;
    }
}

void file_stream::seek(seek_mode mode, long long offset)
{
    const auto fileHandle = reinterpret_cast<HANDLE>(handle_);

    LARGE_INTEGER loffset, lcurPos;
    loffset.QuadPart = offset;

    if (!SetFilePointerEx(
        fileHandle,
        loffset,
        &lcurPos,
        get_win32_seek_mode_(mode)))
    {
        throw std::system_error(GetLastError(), std::generic_category());
    }
    
    pos_ = lcurPos.QuadPart;
}

void file_stream::jump_to(unsigned long long pos)
{
    if (pos > LLONG_MAX) // TODO: Mark unlikely
    {
        throw std::runtime_error(
            "Jumping to file positions greater than LLONG_MAX is not supported on Windows"
        );
    }

    const auto fileHandle = reinterpret_cast<HANDLE>(handle_);

    LARGE_INTEGER loffset, lcurPos;
    loffset.QuadPart = pos;

    if (!SetFilePointerEx(
        fileHandle,
        loffset,
        &lcurPos,
        FILE_BEGIN))
    {
        throw std::system_error(GetLastError(), std::generic_category());
    }
    
    pos_ = lcurPos.QuadPart;
}

bool file_stream::try_reopen(const char* filePath, unsigned long flags)
{
    // TODO: Handle the case of trying to reopen a handle to the same file differently?

    // Attempt to open a new handle.
    const auto newHandle = try_open_(filePath, flags);

    if (reinterpret_cast<HANDLE>(newHandle) == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // Close any existing handles.
    if (reinterpret_cast<HANDLE>(handle_) != INVALID_HANDLE_VALUE)
    {
        try_close_(handle_);
    }

    caps_ = determine_capabilities_(flags);
    pos_ = 0;
    handle_ = newHandle;

    return true;
}

bool file_stream::try_close() noexcept
{
    if (reinterpret_cast<HANDLE>(handle_) == INVALID_HANDLE_VALUE)
    {
        return true;
    }

    const bool result = try_close_(handle_);

    caps_ = stream_capabilities::NONE;
    pos_ = 0;
    handle_ = reinterpret_cast<std::uintmax_t>(INVALID_HANDLE_VALUE);

    return result;
}

void file_stream::close()
{
    if (!try_close())
    {
        throw std::system_error(GetLastError(), std::generic_category());
    }
}

std::uintmax_t file_stream::release() noexcept
{
    const auto r = handle_;

    caps_ = stream_capabilities::NONE;
    pos_ = 0;
    handle_ = reinterpret_cast<std::uintmax_t>(INVALID_HANDLE_VALUE);

    return r;
}

file_stream::file_stream() noexcept
    : stream(stream_capabilities::NONE)
    , handle_(reinterpret_cast<std::uintmax_t>(INVALID_HANDLE_VALUE))
{
}

file_stream::file_stream(
    std::uintmax_t nativeHandle,
    bool canRead,
    bool canWrite) noexcept
    : stream(determine_capabilities_(canRead, canWrite))
    , handle_(nativeHandle)
{
    // HACK: Set the pos_ field by seeking to the current position.
    // NOTE: Win32 doesn't seem to have a tell function for HANDLE
    // file descriptors, so this seems to be the best way.
    seek(seek_mode::current, 0);
}

file_stream::file_stream(const char* filePath, unsigned long flags)
    : stream(determine_capabilities_(flags))
    , handle_(try_open_(filePath, flags))
{
}

file_stream::~file_stream()
{
    if (reinterpret_cast<HANDLE>(handle_) != INVALID_HANDLE_VALUE)
    {
        try_close_(handle_);
    }
}
}
