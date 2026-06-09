/// @file rad_stream.cpp
/// @author Graham Scott
/// @brief TODO
/// @date 2024-07-05
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#include "rad_stream.h"
#include "rad_vector.h"
#include <cstring>

namespace rad
{
static const unsigned char stream_nulls_static_buffer_[1024] = {};

static const char stream_generic_read_error_message_[] =
    "Could not read the requested number of bytes from the stream";

static const char stream_generic_write_error_message_[] =
    "Could not write the requested number of bytes to the stream";

static const char stream_string_read_error_message_[] =
    "Could not read the requested string from the stream in its entirety";

static constexpr std::size_t stream_string8_batch_size_ = 32;

bool stream::try_read_string8_in_batches_(
    char* buf,
    char* bufEnd,
    std::size_t& totalReadCharCount)
{
    char batchBuf[stream_string8_batch_size_];
    char* ptr = buf;

    while (true)
    {
        // Try to fill batchBuf with bytes; break early if we can not read any bytes.
        const auto batchSize = std::min<std::size_t>(
            bufEnd - ptr,
            stream_string8_batch_size_
        );

        if (batchSize == 0)
        {
            break;
        }

        const auto readCharCount = try_read(batchBuf, batchSize);

        if (readCharCount == 0)
        {
            break;
        }

        // Check buf for any null-terminator characters.
        const auto nullTerminatorChar = static_cast<const char*>(
            std::memchr(batchBuf, '\0', readCharCount)
        );

        if (nullTerminatorChar)
        {
            // Append the characters from before the null-terminator, and the null-terminator itself.
            const std::size_t validCharCount = (nullTerminatorChar - batchBuf) + 1;

            std::memcpy(ptr, batchBuf, validCharCount);
            ptr += validCharCount;

            // Go backwards in the stream if necessary (i.e. we read past the end of the string).
            if (validCharCount < readCharCount)
            {
                jump_behind(readCharCount - validCharCount);
            }

            totalReadCharCount = static_cast<std::size_t>(ptr - buf);
            return true;
        }
        else
        {
            // Append all of the characters in the batch, since
            // we have not yet reached the null-terminator.
            std::memcpy(ptr, batchBuf, readCharCount);
            ptr += readCharCount;
        }
    }
    
    totalReadCharCount = static_cast<std::size_t>(ptr - buf);
    return false;
}

bool stream::try_read_string8_one_by_one_(
    char* buf,
    char* bufEnd,
    std::size_t& totalReadCharCount)
{
    char ch;
    char* ptr = buf;

    while (ptr != bufEnd && try_read(&ch, 1))
    {
        *(ptr++) = ch;

        if (ch == '\0')
        {
            totalReadCharCount = static_cast<std::size_t>(ptr - buf);
            return true;
        }
    }
    
    totalReadCharCount = static_cast<std::size_t>(ptr - buf);
    return false;
}

bool stream::try_read_string8_in_batches_(vector<char>& buf)
{
    while (true)
    {
        // Append a new batch.
        const auto ptr = buf.append(no_value_init, stream_string8_batch_size_);

        // Try to fill buffer with bytes; break early if we could not read any bytes.
        const auto readCharCount = try_read(ptr, stream_string8_batch_size_);

        if (readCharCount == 0)
        {
            break;
        }

        // Check buffer for any null-terminator characters.
        const auto nullTerminatorChar = static_cast<const char*>(
            std::memchr(ptr, '\0', readCharCount)
        );

        if (nullTerminatorChar)
        {
            // Append the characters from before the null-terminator, and the null-terminator itself.
            const std::size_t validCharCount = (nullTerminatorChar - ptr) + 1;

            // Go backwards in the stream if necessary (i.e. we read past the end of the string).
            if (validCharCount < readCharCount)
            {
                jump_behind(readCharCount - validCharCount);
                buf.pop_back(stream_string8_batch_size_ - validCharCount);
            }

            return true;
        }
    }
    
    return false;
}

bool stream::try_read_string8_one_by_one_(vector<char>& buf)
{
    char ch;

    while (try_read(&ch, 1))
    {
        buf.push_back(ch);

        if (ch == '\0')
        {
            return true;
        }
    }
    
    return false;
}

unsigned long long stream::get_size() const
{
    assert(caps_.can_get_size() &&
        "This stream does not have the capability to get its size"
    );

    return 0;
}

const void* stream::get_data_pointer() const noexcept
{
    assert(caps_.can_get_data_pointer() &&
        "This stream does not have the capability to get its data pointer"
    );

    return nullptr;
}

void stream::flush()
{
    assert(caps_.can_write() &&
        "This stream does not have the capability to write"
    );
}

std::size_t stream::try_read(void* buf, std::size_t size)
{
    assert(caps_.can_read() &&
        "This stream does not have the capability to read"
    );

    return 0;
}

std::size_t stream::try_write(const void* buf, std::size_t size)
{
    assert(caps_.can_write() &&
        "This stream does not have the capability to write"
    );

    return 0;
}

void stream::seek(seek_mode mode, long long offset)
{
    assert(caps_.can_seek() &&
        "This stream does not have the capability to seek"
    );
}

void stream::jump_to(unsigned long long pos)
{
    assert(caps_.can_seek() &&
        "This stream does not have the capability to seek"
    );
}

void stream::jump_ahead(unsigned long long amount)
{
    if (caps_.can_seek())
    {
        jump_to(pos_ + amount);
    }
    else
    {
        unsigned char ch;
        while (amount && try_read(&ch, 1))
        {
            --amount;
        }
    }
}

std::size_t stream::align(std::size_t stride)
{
    // If stride is < 2, we don't need to align.
    // TODO: Is this check necessary? If not it's almost surely faster to just remove it.
    if (stride-- < 2)
    {
        return 0;
    }

    // Compute the closest position in the stream that's aligned
    // by the given stride, and jump to that position.
    const auto oldPos = pos_;
    const unsigned long long newPos = (
        (oldPos + stride) & static_cast<unsigned long long>(~stride)
    );

    jump_to(newPos);

    return (newPos - oldPos);
}

std::size_t stream::pad(std::size_t stride)
{
    // If stride is < 2, we don't need to pad.
    if (stride-- < 2)
    {
        return 0;
    }

    // Compute the amount of nulls we need to write to align the
    // stream with the given stride, and write that many nulls.
    const auto oldPos = pos_;
    const unsigned long long newPos = (
        (oldPos + stride) & static_cast<unsigned long long>(~stride)
    );

    const auto nullsCount = static_cast<std::size_t>(newPos - oldPos);

    write_nulls(nullsCount);

    return nullsCount;
}

std::size_t stream::try_write_nulls(std::size_t amount)
{
    if (amount <= std::size(stream_nulls_static_buffer_))
    {
        return try_write(stream_nulls_static_buffer_, amount);
    }
    else
    {
        // Try to write all of the requested nulls, one chunk at a time.
        std::size_t bytesLeftToWrite = amount;

        do
        {
            const auto writtenBytes = try_write(
                stream_nulls_static_buffer_,
                std::size(stream_nulls_static_buffer_)
            );

            if (writtenBytes == 0)
            {
                return (amount - bytesLeftToWrite);
            }

            bytesLeftToWrite -= writtenBytes;
        }
        while (bytesLeftToWrite > std::size(stream_nulls_static_buffer_));

        // Write the final chunk of nulls.
        bytesLeftToWrite -= try_write(
            stream_nulls_static_buffer_,
            bytesLeftToWrite
        );

        return (amount - bytesLeftToWrite);
    }
}

bool stream::try_read_string8(
    char* buf,
    std::size_t bufSize,
    std::size_t& readCharCount)
{
    const auto bufEnd = buf + bufSize;

    return (caps_.can_seek() && !caps_.can_nocost_read()) ?
        try_read_string8_in_batches_(buf, bufEnd, readCharCount) :
        try_read_string8_one_by_one_(buf, bufEnd, readCharCount);
}

bool stream::try_read_string8(vector<char>& buf)
{
    return (caps_.can_seek() && !caps_.can_nocost_read()) ?
        try_read_string8_in_batches_(buf) :
        try_read_string8_one_by_one_(buf);
}

void stream::write_nulls(std::size_t amount)
{
    if (try_write_nulls(amount) != amount)
    {
        throw std::runtime_error(stream_generic_write_error_message_);
    }
}

std::size_t stream::read_string8(char* buf, std::size_t bufSize)
{
    std::size_t readCharCount;

    if (!try_read_string8(buf, bufSize, readCharCount))
    {
        throw std::runtime_error(stream_string_read_error_message_);
    }

    return readCharCount;
}

std::size_t stream::read_string8(vector<char>& buf)
{
    const auto bufStartSize = buf.size();

    if (!try_read_string8(buf))
    {
        throw std::runtime_error(stream_string_read_error_message_);
    }

    return buf.size() - bufStartSize;
}

string stream::read_string8(allocator& allocator)
{
    vector<char> strMem(allocator);

    if (!try_read_string8(strMem))
    {
        throw std::runtime_error(stream_string_read_error_message_);
    }

    if (strMem.size() > string::max_size()) // TODO: Mark unlikely
    {
        throw std::length_error("String length exceeds string::max_size()");
    }

    return string{take_ownership, allocator, strMem.release()};
}

void stream::read(void* buf, std::size_t size)
{
    if (try_read(buf, size) != size)
    {
        throw std::runtime_error(stream_generic_read_error_message_);
    }
}

void stream::write(const void* buf, std::size_t size)
{
    if (try_write(buf, size) != size)
    {
        throw std::runtime_error(stream_generic_write_error_message_);
    }
}
}
