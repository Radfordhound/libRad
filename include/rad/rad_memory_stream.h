/// @file rad_memory_stream.h
/// @author Graham Scott
/// @brief TODO
/// @date 2024-12-18
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_MEMORY_STREAM_H_INCLUDED
#define RAD_MEMORY_STREAM_H_INCLUDED

#include "rad_stream.h"
#include "rad_span.h"
#include "rad_vector.h"
#include <cstddef>

namespace rad
{
class readonly_memory_stream
    : public stream
{
    static constexpr stream_capabilities static_capabilities_ =
        stream_capabilities::CAPS1_CAN_SEEK |
        stream_capabilities::CAPS1_CAN_READ |
        stream_capabilities::CAPS1_CAN_GET_SIZE |
        stream_capabilities::CAPS1_CAN_GET_DATA_POINTER |
        stream_capabilities::CAPS2_CAN_NOCOST_SEEK |
        stream_capabilities::CAPS2_CAN_NOCOST_READ;

    span<const unsigned char>   data_;

public:
    inline span<const unsigned char> data() const noexcept
    {
        return data_;
    }

    inline unsigned long long get_size() const override
    {
        return data_.size();
    }

    inline const void* get_data_pointer() const noexcept override
    {
        return data_.data() + pos_;
    }

    RAD_API std::size_t try_read(void* buf, std::size_t size) override;

    RAD_API void seek(seek_mode mode, long long offset) override;

    RAD_API void jump_to(unsigned long long pos) override;

    RAD_API bool try_read_string8(
        char* buf,
        std::size_t bufSize,
        std::size_t& readCharCount
    ) override;

    RAD_API bool try_read_string8(vector<char>& buf) override;

    RAD_API readonly_memory_stream() noexcept;

    RAD_API explicit readonly_memory_stream(span<const unsigned char> data) noexcept;

    RAD_API readonly_memory_stream(
        const void* data,
        std::size_t size
    ) noexcept;
};

class memory_stream
    : public stream
{
    static constexpr stream_capabilities static_capabilities_ =
        stream_capabilities::CAPS1_CAN_SEEK_READ_WRITE |
        stream_capabilities::CAPS1_CAN_GET_SIZE |
        stream_capabilities::CAPS1_CAN_GET_DATA_POINTER |
        stream_capabilities::CAPS2_CAN_NOCOST_SEEK_READ_WRITE;

    vector<unsigned char>   data_;

public:
    inline span<const unsigned char> data() const noexcept
    {
        return data_;
    }

    inline span<unsigned char> data() noexcept
    {
        return data_;
    }

    inline void reserve(std::size_t newCapacity)
    {
        data_.reserve(newCapacity);
    }

    RAD_API void clear() noexcept;

    [[nodiscard]] inline vector<unsigned char> release() noexcept
    {
        pos_ = 0;
        return std::move(data_);
    }

    inline unsigned long long get_size() const override
    {
        return data_.size();
    }

    inline const void* get_data_pointer() const noexcept override
    {
        return data_.data() + pos_;
    }

    RAD_API std::size_t try_read(void* buf, std::size_t size) override;

    RAD_API std::size_t try_write(const void* buf, std::size_t size) override;

    RAD_API void seek(seek_mode mode, long long offset) override;

    RAD_API void jump_to(unsigned long long pos) override;

    RAD_API bool try_read_string8(
        char* buf,
        std::size_t bufSize,
        std::size_t& readCharCount
    ) override;

    RAD_API bool try_read_string8(vector<char>& buf) override;

    RAD_API explicit memory_stream(
        rad::allocator& allocator = rad::default_allocator
    ) noexcept;

    RAD_API explicit memory_stream(span<const unsigned char> data);

    RAD_API memory_stream(
        const void* data,
        std::size_t size
    );

    RAD_API explicit memory_stream(vector<unsigned char> data);
};
}

#endif

