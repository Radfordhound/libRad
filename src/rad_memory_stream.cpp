/// @file rad_memory_stream.cpp
/// @author Graham Scott
/// @brief TODO
/// @date 2024-12-18
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_memory_stream.h"

namespace rad
{
std::size_t readonly_memory_stream::try_read(void* buf, std::size_t size)
{
    // Cap requested read size.
    const std::size_t bytesLeft = data_.size() - pos_;
    if (size > bytesLeft)
    {
        size = bytesLeft;
    }

    // Copy data to buffer.
    std::memcpy(buf, data_.data() + pos_, size);
    pos_ += size;

    return size;
}

void readonly_memory_stream::seek(seek_mode mode, long long offset)
{
    unsigned long long pos;

    switch (mode)
    {
    case seek_mode::current:
        pos = pos_;
        break;

    case seek_mode::end:
        pos = data_.size();
        break;

    case seek_mode::begin:
    default:
        pos = offset;
        break;
    }

    if ((offset >= 0 && pos + offset > data_.size()) || (offset < 0 && -offset > pos))
    {
        throw std::runtime_error("Attempted to seek outside of valid stream range");
    }

    pos_ = pos;
}

void readonly_memory_stream::jump_to(unsigned long long pos)
{
    if (pos > data_.size())
    {
        throw std::runtime_error("Attempted to seek outside of valid stream range");
    }

    pos_ = pos;
}

bool readonly_memory_stream::try_read_string8(
    char* buf,
    std::size_t bufSize,
    std::size_t& readCharCount)
{
    assert(data_.data() != nullptr &&
        "Attempted to read a string from a readonly_memory_stream "
        "which has no data!"
    );

    const auto bytesLeft = static_cast<std::size_t>(data_.size() - pos_);
    const auto strPtr = reinterpret_cast<const char*>(data_.data() + pos_);

    const auto nullTerminatorPtr = static_cast<const char*>(
        std::memchr(strPtr, '\0', bytesLeft)
    );

    if (!nullTerminatorPtr)
    {
        const auto strSize = (static_cast<std::size_t>(nullTerminatorPtr - strPtr) + 1);
        if (bufSize >= strSize)
        {
            std::memcpy(buf, strPtr, strSize);

            pos_ += strSize;
            readCharCount = strSize;

            return true;
        }
    }

    readCharCount = 0;
    return false;
}

bool readonly_memory_stream::try_read_string8(vector<char>& buf)
{
    const auto bytesLeft = static_cast<std::size_t>(data_.size() - pos_);
    const auto strPtr = reinterpret_cast<const char*>(data_.data() + pos_);

    const auto nullTerminatorPtr = static_cast<const char*>(
        std::memchr(strPtr, '\0', bytesLeft)
    );

    if (!nullTerminatorPtr) return false;

    buf.append(strPtr, nullTerminatorPtr + 1);
    return true;
}

readonly_memory_stream::readonly_memory_stream() noexcept
    : stream(stream_capabilities::NONE)
{
}

readonly_memory_stream::readonly_memory_stream(
    span<const unsigned char> data) noexcept
    : stream(static_capabilities_)
    , data_(data)
{
}

readonly_memory_stream::readonly_memory_stream(
    const void* data,
    std::size_t size) noexcept
    : stream(static_capabilities_)
    , data_(static_cast<const unsigned char*>(data), size)
{
}

void memory_stream::clear() noexcept
{
    data_.clear();
    pos_ = 0;
}

std::size_t memory_stream::try_read(void* buf, std::size_t size)
{
    // Cap requested read size.
    const std::size_t bytesLeft = data_.size() - pos_;
    if (size > bytesLeft)
    {
        size = bytesLeft;
    }

    // Copy data to buffer.
    std::memcpy(buf, data_.data() + pos_, size);
    pos_ += size;

    return size;
}

std::size_t memory_stream::try_write(const void* buf, std::size_t size)
{
    // Insert data at current position in stream.

    // TODO: Do we want to override data actually??
    // For example: should 1234 become 12ABC instead of 12ABC34 ?
    //                       ^

    auto srcData = static_cast<const unsigned char*>(buf);
    const auto srcDataEnd = srcData + size;
    auto it = data_.begin() + pos_;

    if (it != data_.end())
    {
        const auto maxOverwriteCount = static_cast<std::size_t>(data_.end() - it);

        if (size <= maxOverwriteCount)
        {
            std::memcpy(it, srcData, size);
            pos_ += size;
            return size;
        }

        std::memcpy(it, srcData, maxOverwriteCount);
        it += maxOverwriteCount;
        srcData += maxOverwriteCount;
    }

    data_.insert(it, srcData, srcDataEnd);
    pos_ += size;

    return size;
}

void memory_stream::seek(seek_mode mode, long long offset)
{
    unsigned long long pos;

    switch (mode)
    {
    case seek_mode::current:
        pos = pos_;
        break;

    case seek_mode::end:
        pos = data_.size();
        break;

    case seek_mode::begin:
    default:
        pos = offset;
        break;
    }

    if ((offset >= 0 && pos + offset > data_.size()) || (offset < 0 && -offset > pos))
    {
        throw std::runtime_error("Attempted to seek outside of valid stream range");
    }

    pos_ = pos;
}

void memory_stream::jump_to(unsigned long long pos)
{
    if (pos > data_.size())
    {
        throw std::runtime_error("Attempted to seek outside of valid stream range");
    }

    pos_ = pos;
}

bool memory_stream::try_read_string8(
    char* buf,
    std::size_t bufSize,
    std::size_t& readCharCount)
{
    const auto bytesLeft = static_cast<std::size_t>(data_.size() - pos_);
    const auto strPtr = reinterpret_cast<const char*>(data_.data() + pos_);

    const auto nullTerminatorPtr = static_cast<const char*>(
        std::memchr(strPtr, '\0', bytesLeft)
    );

    if (!nullTerminatorPtr)
    {
        const auto strSize = (static_cast<std::size_t>(nullTerminatorPtr - strPtr) + 1);
        if (bufSize >= strSize)
        {
            std::memcpy(buf, strPtr, strSize);

            pos_ += strSize;
            readCharCount = strSize;

            return true;
        }
    }

    readCharCount = 0;
    return false;
}

bool memory_stream::try_read_string8(vector<char>& buf)
{
    const auto bytesLeft = static_cast<std::size_t>(data_.size() - pos_);
    const auto strPtr = reinterpret_cast<const char*>(data_.data() + pos_);

    const auto nullTerminatorPtr = static_cast<const char*>(
        std::memchr(strPtr, '\0', bytesLeft)
    );

    if (!nullTerminatorPtr) return false;

    buf.append(strPtr, nullTerminatorPtr + 1);
    return true;
}

memory_stream::memory_stream() noexcept
    : stream(static_capabilities_)
{
}

memory_stream::memory_stream(span<const unsigned char> data)
    : stream(static_capabilities_)
    , data_(data.begin(), data.end())
{
}

memory_stream::memory_stream(
    const void* data,
    std::size_t size)
    : stream(static_capabilities_)
    , data_(
        static_cast<const unsigned char*>(data),
        static_cast<const unsigned char*>(data) + size
    )
{
}

memory_stream::memory_stream(vector<unsigned char> data)
    : stream(static_capabilities_)
    , data_(std::move(data))
{
}
}
