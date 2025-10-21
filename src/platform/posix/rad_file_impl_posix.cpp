/// @file rad_file_impl_posix.cpp
/// @author Graham Scott
/// @brief TODO
/// @date 2024-07-13
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_file.h"
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

namespace rad
{
static constexpr unsigned long guaranteed_caps_ = (
    stream_capabilities::CAPS1_CAN_SEEK |
    stream_capabilities::CAPS1_CAN_GET_SIZE
);

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
    int oflag = 0;

    // Get open flags.
    switch (flags & OPEN_MASK_MODE)
    {
    case OPEN_MODE_READ_ONLY:
        oflag |= O_RDONLY;
        break;

    case OPEN_MODE_WRITE_ONLY:
        oflag |= O_WRONLY;
        break;

    case OPEN_MODE_READ_WRITE:
        oflag |= O_RDWR;
        break;
    }

    switch (flags & OPEN_MASK_MODE)
    {
    case OPEN_MODE_WRITE_ONLY:
    case OPEN_MODE_READ_WRITE:
        if (flags & OPEN_FLAG_CREATE)
        {
            oflag |= O_CREAT | O_TRUNC;
        }
        else
        {
            oflag |= O_APPEND | O_EXCL;
        }
        break;
    }

    // TODO: Pass third permissions argument to open.
    const auto fileHandle = ::open(filePath, oflag);

    if (fileHandle == -1)
    {
        // TODO: Throw an exception instead if the error is not related to file not existing!
        return UINTMAX_MAX;
    }

    return fileHandle;
}

bool file_stream::try_close_(std::uintmax_t handle) noexcept
{
    return (::close(static_cast<int>(handle)) == 0);
}

std::size_t file_stream::try_read_(void* buf, std::size_t size)
{
    const auto fileHandle = static_cast<int>(handle_);
    const auto bytesReadOrError = ::read(fileHandle, buf, size);

    if (bytesReadOrError == -1)
    {
        const int lastErr = errno;

        // In error cases where the file descriptor is guaranteed by the
        // POSIX specification to not be modified, just return 0;
        if (lastErr == EAGAIN) // TODO: Are there other cases where POSIX guarantees this?
        {
            return 0;
        }

        // In other cases, we need to inform the user that the file position
        // is now, unfortunately, indeterminate. So, we throw an exception.
        throw std::system_error(errno, std::generic_category());
    }

    const auto bytesRead = static_cast<std::size_t>(bytesReadOrError);

    pos_ += bytesRead;
    return bytesRead;
}

unsigned long long file_stream::get_size() const
{
    // TODO
    return 0;
}

void file_stream::flush()
{
    assert(caps_.can_write() &&
        "This stream does not have the capability to write"
    );

    // TODO
}

std::size_t file_stream::try_read(void* buf, std::size_t size)
{
    assert(caps_.can_read() &&
        "This stream does not have the capability to read"
    );

    const auto dstBegin = static_cast<unsigned char*>(buf);
    auto curDst = dstBegin;

    // TODO: Read in chunks of 4MiB instead??

    // POSIX read() has implementation-defined results if we request a number of bytes
    // that is greater than SSIZE_MAX; so if the requested size is greater than this
    // (very unlikely), we first read in chunks of up to SSIZE_MAX bytes.
    while (size > SSIZE_MAX) // TODO: Mark unlikely
    {
        const auto bytesRead = try_read_chunk_(curDst, SSIZE_MAX);
        curDst += bytesRead;

        if (bytesRead < SSIZE_MAX)
        {
            return static_cast<std::size_t>(curDst - dstBegin);
        }

        size -= bytesRead;
    }

    // Read the final chunk (most of the time this will be the only actual call to try_read_chunk_).
    curDst += try_read_chunk_(curDst, size);

    return static_cast<std::size_t>(curDst - dstBegin);
}

std::size_t file_stream::try_write(const void* buf, std::size_t size)
{
    assert(caps_.can_write() &&
        "This stream does not have the capability to write"
    );

    // TODO
    return 0;
}

static constexpr int get_posix_seek_mode_(seek_mode mode) noexcept
{
    switch (mode)
    {
    case seek_mode::current:
        return SEEK_CUR;
    
    case seek_mode::end:
        return SEEK_END;

    default:
        return SEEK_SET;
    }
}

void file_stream::seek(seek_mode mode, long long offset)
{
    using posix_off_type_ = decltype(lseek(0, 0, 0));

    static_assert(sizeof(posix_off_type_) >= sizeof(long long),
        "sizeof(off_t) must be >= sizeof(unsigned long long) "
        "for file_stream::seek to work correctly in all cases"
    );

    const auto fileHandle = static_cast<int>(handle_);
    const int whence = get_posix_seek_mode_(mode);
    const auto newPos = lseek(fileHandle, offset, whence);

    if (newPos == static_cast<posix_off_type_>(-1))
    {
        throw std::system_error(errno, std::generic_category());
    }
    
    pos_ = static_cast<unsigned long long>(newPos);
}

void file_stream::jump_to(unsigned long long pos)
{
    using posix_off_type_ = decltype(lseek(0, 0, 0));

    static_assert(sizeof(posix_off_type_) >= sizeof(unsigned long long),
        "sizeof(off_t) must be >= sizeof(unsigned long long) "
        "for file_stream::jump_to to work correctly in all cases"
    );

    const auto fileHandle = static_cast<int>(handle_);
    const auto newPos = lseek(fileHandle, pos, SEEK_SET);

    if (newPos == static_cast<posix_off_type_>(-1))
    {
        throw std::system_error(errno, std::generic_category());
    }
    
    pos_ = static_cast<unsigned long long>(newPos);
}

bool file_stream::try_reopen(const char* filePath, unsigned long flags)
{
    // TODO: Handle the case of trying to reopen a handle to the same file differently?

    // Attempt to open a new handle.
    const auto newHandle = try_open_(filePath, flags);

    if (newHandle == UINTMAX_MAX)
    {
        return false;
    }

    // Close any existing handles.
    if (handle_ != UINTMAX_MAX)
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
    if (handle_ == UINTMAX_MAX)
    {
        return true;
    }

    const bool result = try_close_(handle_);

    caps_ = stream_capabilities::NONE;
    pos_ = 0;
    handle_ = UINTMAX_MAX;

    return result;
}

void file_stream::close()
{
    if (!try_close())
    {
        throw std::system_error(errno, std::generic_category());
    }
}

std::uintmax_t file_stream::release() noexcept
{
    const auto r = handle_;

    caps_ = stream_capabilities::NONE;
    pos_ = 0;
    handle_ = UINTMAX_MAX;

    return r;
}

file_stream::file_stream() noexcept
    : stream(stream_capabilities::NONE)
    , handle_(UINTMAX_MAX)
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
    // NOTE: POSIX doesn't seem to have a tell function for int
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
    if (handle_ != UINTMAX_MAX)
    {
        try_close_(handle_);
    }
}
}
