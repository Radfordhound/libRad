/// @file rad_stream.h
/// @author Graham Scott
/// @brief TODO
/// @date 2024-07-05
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_STREAM_H_INCLUDED
#define RAD_STREAM_H_INCLUDED

#include "rad_base.h"
#include "rad_allocator.h"
#include "rad_string.h"
#include <cstddef>

namespace rad
{
template<typename T>
class vector;

struct stream_capabilities
{
    enum flags
    {
        NONE = 0,

        CAPS1_CAN_SEEK = 1,
        CAPS1_CAN_READ = 2,
        CAPS1_CAN_WRITE = 4,
        CAPS1_CAN_GET_SIZE = 8,
        CAPS1_CAN_GET_DATA_POINTER = 16,

        CAPS1_CAN_SEEK_READ_WRITE = (
            CAPS1_CAN_SEEK |
            CAPS1_CAN_READ |
            CAPS1_CAN_WRITE
        ),

        CAPS2_CAN_NOCOST_SEEK = 0x100,
        CAPS2_CAN_NOCOST_READ = 0x200,
        CAPS2_CAN_NOCOST_WRITE = 0x400,

        CAPS2_CAN_NOCOST_SEEK_READ_WRITE = (
            CAPS2_CAN_NOCOST_SEEK |
            CAPS2_CAN_NOCOST_READ |
            CAPS2_CAN_NOCOST_WRITE
        ),
    };

    unsigned long flags1;

    constexpr bool can_seek() const noexcept
    {
        return (flags1 & CAPS1_CAN_SEEK);
    }

    constexpr bool can_read() const noexcept
    {
        return (flags1 & CAPS1_CAN_READ);
    }

    constexpr bool can_write() const noexcept
    {
        return (flags1 & CAPS1_CAN_WRITE);
    }

    constexpr bool can_get_size() const noexcept
    {
        return (flags1 & CAPS1_CAN_GET_SIZE);
    }

    constexpr bool can_get_data_pointer() const noexcept
    {
        return (flags1 & CAPS1_CAN_GET_DATA_POINTER);
    }

    constexpr bool can_nocost_seek() const noexcept
    {
        return (flags1 & CAPS2_CAN_NOCOST_SEEK);
    }

    constexpr bool can_nocost_read() const noexcept
    {
        return (flags1 & CAPS2_CAN_NOCOST_READ);
    }

    constexpr bool can_nocost_write() const noexcept
    {
        return (flags1 & CAPS2_CAN_NOCOST_WRITE);
    }

    constexpr stream_capabilities(unsigned long flags1) noexcept
        : flags1(flags1)
    {
    }
};

class stream
{
protected:
    stream_capabilities caps_;
    unsigned long long  pos_ = 0;

    bool try_read_string8_in_batches_(
        char* buf,
        char* bufEnd,
        std::size_t& totalReadCharCount
    );

    bool try_read_string8_one_by_one_(
        char* buf,
        char* bufEnd,
        std::size_t& totalReadCharCount
    );

    bool try_read_string8_in_batches_(vector<char>& buf);

    bool try_read_string8_one_by_one_(vector<char>& buf);

    constexpr stream(stream_capabilities capabilities) noexcept
        : caps_(capabilities)
    {
    }

public:
    enum class seek_mode
    {
        /// @brief Seek starting from the beginning of the stream.
        begin,
        /// @brief Seek starting from the current position in the stream.
        current,
        /// @brief Seek starting from the end of the stream.
        end,
    };

    constexpr stream_capabilities capabilities() const noexcept
    {
        return caps_;
    }

    inline unsigned long long tell() const noexcept
    {
        return pos_;
    }
    
    virtual ~stream() = default;

    /// @brief Retrieves the total number of bytes currently in the stream.
    ///
    /// NOTE: It is undefined behavior to call this function if the stream
    /// does not have the capability to get its size.
    ///
    /// @return The total number of bytes currently in the stream.
    RAD_API virtual unsigned long long get_size() const;

    /// @brief Retrieves the pointer to the current byte
    /// within the stream's data array.
    ///
    /// NOTE: It is undefined behavior to call this function if the stream
    /// does not have the capability to get its data pointer.
    ///
    /// @return The pointer to the current byte within the stream's
    /// data array, or nullptr if the stream does not have the
    /// capability to get a data pointer.
    RAD_API virtual const void* get_data_pointer() const noexcept;

    /// @brief Flushes any pending writes made to the stream, if necessary.
    ///
    /// NOTE: It is undefined behavior to call this function if the stream
    /// does not have the capability to write.
    RAD_API virtual void flush();

    /// @brief Attempts to read the requested number of bytes
    /// into the given buffer (as unsigned chars).
    ///
    /// NOTE: It is undefined behavior to call this function if the stream
    /// does not have the capability to read.
    ///
    /// @details Normally, this function will not throw an exception, and
    /// will simply return the number of bytes that were successfully read
    /// from the stream into the given buffer, which may be less than the
    /// requested number in typical error/end-of-stream cases.
    ///
    /// However, in some exceptional error cases, this function may not be
    /// able to handle this cleanly, and instead, it will throw a
    /// `std::system_error` exception.
    ///
    /// If this happens, it means that, from now on, the position of the
    /// stream unfortunately can no longer be specified, and the value
    /// returned by `tell()` may be incorrect, and should be assumed as such.
    ///
    /// Therefore, in order to continue using the stream after an exception
    /// is thrown by this function, you must first correct the position manually
    /// using `jump_to` (or via `seek` with `seek_mode::begin`), in order to
    /// set it to a known state.
    ///
    /// This works unless the stream is not seekable, in which case, your best
    /// course of action at this point would be to attempt to reopen the stream.
    ///
    /// This is unfortunately unavoidable, as this is the behavior of several
    /// operating systems and standards (such as POSIX's `read` and the
    /// C/C++ standard library's `fread`).
    ///
    /// @param buf The buffer to read the bytes into (as unsigned chars).
    /// @param size The number of bytes to read from the stream.
    /// @return The number of bytes that were successfully read
    /// from the stream into the given buffer.
    virtual std::size_t try_read(void* buf, std::size_t size);

    /// @brief Attempts to write the requested number of unsigned chars
    /// to the stream as bytes, from the given buffer.
    ///
    /// NOTE: It is undefined behavior to call this function if the stream
    /// does not have the capability to write.
    ///
    /// @details Normally, this function will not throw an exception, and
    /// will simply return the number of bytes that were successfully
    /// written to the stream, which may be less than the requested
    /// number in typical error/end-of-stream cases.
    ///
    /// However, in some exceptional error cases, this function may not be
    /// able to handle this cleanly, and instead, it will throw a
    /// `std::system_error` exception.
    ///
    /// If this happens, it means that, from now on, the position of the
    /// stream unfortunately can no longer be specified, and the value
    /// returned by `tell()` may be incorrect, and should be assumed as such.
    ///
    /// Therefore, in order to continue using the stream after an exception
    /// is thrown by this function, you must first correct the position manually
    /// using `jump_to` (or via `seek` with `seek_mode::begin`), in order to
    /// set it to a known state.
    ///
    /// This works unless the stream is not seekable, in which case, your best
    /// course of action at this point would be to attempt to reopen the stream.
    ///
    /// This is unfortunately unavoidable, as this is the behavior of several
    /// operating systems and standards (such as POSIX's `write` and the
    /// C/C++ standard library's `fwrite`).
    ///
    /// @param buf The buffer of unsigned chars to write to the stream as bytes.
    /// @param size The number of bytes to write to the stream.
    /// @return The number of bytes that were successfully written to the stream.
    virtual std::size_t try_write(const void* buf, std::size_t size);

    /// @brief Performs a **relative** jump from the start position
    /// specified by the seek_mode to the given offset.
    ///
    /// NOTE: It is undefined behavior to call this function if the stream
    /// does not have the capability to seek.
    ///
    /// @param mode The seek mode; specifies the position that the seek should start from.
    /// @param offset The offset to add to the start position specified by the seek mode.
    virtual void seek(seek_mode mode, long long offset);

    /// @brief Performs an **absolute** jump to the given position.
    ///
    /// NOTE: It is undefined behavior to call this function if the stream
    /// does not have the capability to seek.
    ///
    /// @details This function is **not** necessarily equivalent to
    /// `seek(seek_mode::current, pos)`, because it takes an **unsigned**
    /// integer value for its position value, allowing more range, unlike
    /// seek which takes a **signed** integer value. Thus, it requires a
    /// different implementation.
    ///
    /// @param pos The absolute position to jump to.
    virtual void jump_to(unsigned long long pos);

    /// @brief Skips ahead in the stream by the given number of bytes.
    ///
    /// NOTE: It is valid to call this function even if the stream does not
    /// have the capability to seek. In that case, it simply falls back to
    /// reading and discarding the bytes.
    ///
    /// @param amount The number of bytes to skip in the stream.
    RAD_API void jump_ahead(unsigned long long amount);

    /// @brief Jump backwards in the stream by the given number of bytes.
    ///
    /// NOTE: It is undefined behavior to call this function if the stream
    /// does not have the capability to seek.
    ///
    /// @param amount The number of bytes to move backwards by within the stream.
    inline void jump_behind(unsigned long long amount)
    {
        jump_to(pos_ - amount);
    }

    RAD_API std::size_t align(std::size_t stride);

    RAD_API std::size_t pad(std::size_t stride);

    RAD_API std::size_t try_write_nulls(std::size_t amount);

    /// @brief Attempts to read a null-terminated 8-bit-char string from
    /// the stream at the current position into the given buffer, including
    /// its null-terminator character.
    ///
    /// NOTE: It is undefined behavior to call this function if the stream
    /// does not have the capability to read.
    ///
    /// @details If the provided buffer size is not large enough, no null-terminator
    /// character is present, or any stream read error occurs, this function will
    /// return false, and the position of the stream and contents of the buffer
    /// are unspecified.
    ///
    /// Otherwise, this function will return true, buf is guaranteed to contain
    /// all characters of the string, including its null-terminator character, and
    /// the position of the stream is guaranteed to correspond to the byte immediately
    /// after the null-terminator character.
    ///
    /// Note that in all cases, the readCharCount variable will be set to the number of
    /// characters that were read from the stream and into the buffer, including
    /// the null-terminator if applicable.
    ///
    /// @param buf The buffer to read the 8-bit characters of the string into.
    /// @param bufSize The size of the given buffer.
    /// @param readCharCount Output variable which will be set to the number of
    /// characters that were read from the stream and into the buffer, including
    /// the null-terminator.
    /// @return True if the entire string was successfully read into the buffer,
    /// including its null-terminator character. False otherwise.
    RAD_API virtual bool try_read_string8(
        char* buf,
        std::size_t bufSize,
        std::size_t& readCharCount
    );

    /// @brief Attempts to read a null-terminated 8-bit-char string from
    /// the stream at the current position and append it to the given buffer,
    /// including its null-terminator character.
    ///
    /// NOTE: It is undefined behavior to call this function if the stream
    /// does not have the capability to read.
    ///
    /// @details If no null-terminator character is present, or any stream read
    /// error occurs, this function will return false, and the position of the
    /// stream and any appended contents of the buffer are unspecified.
    ///
    /// Otherwise, this function will return true, buf is guaranteed to contain
    /// all characters of the string, including its null-terminator character, and
    /// the position of the stream is guaranteed to correspond to the byte immediately
    /// after the null-terminator character.
    ///
    /// Note that in all cases, any new buffer contents will be appended, meaning
    /// that existing buffer contents will not be modified or removed.
    ///
    /// It is always safe to determine how many characters were appended to the
    /// buffer by getting the difference between its size before and after this
    /// function was called, like so: `buf.size() - oldBufSize`.
    ///
    /// @param buf The buffer to read the 8-bit characters of the string into.
    /// @return True if the entire string was successfully read into the buffer,
    /// including its null-terminator character. False otherwise.
    RAD_API virtual bool try_read_string8(vector<char>& buf);

    //RAD_API std::size_t try_read_string8(std::string& str);

    /// @brief Attempts to write a null-terminated 8-bit-char string to the stream.
    ///
    /// NOTE: It is undefined behavior to call this function if the stream
    /// does not have the capability to write.
    ///
    /// @param str The 8-bit-char string to write.
    /// @return The number of bytes that were successfully written to the stream.
    RAD_API std::size_t try_write_string8(const char* str);

    RAD_API void write_nulls(std::size_t amount);

    RAD_API std::size_t read_string8(char* buf, std::size_t bufSize);

    RAD_API std::size_t read_string8(vector<char>& buf);

    RAD_API string read_string8(allocator& allocator = default_allocator);

    /// @brief Writes a null-terminated 8-bit-char string to the stream.
    /// Throws an exception if any of the bytes were not successfully written.
    ///
    /// NOTE: It is undefined behavior to call this function if the stream
    /// does not have the capability to write.
    ///
    /// @param str The 8-bit-char string to write.
    RAD_API void write_string8(const char* str);

    RAD_API void read(void* buf, std::size_t size);

    RAD_API void write(const void* buf, std::size_t size);

    template<typename T>
    inline void read_as(T& obj)
    {
        read(&obj, sizeof(T));
    }

    template<typename T, std::size_t Count>
    inline void read_as(T (&arr)[Count])
    {
        read(&arr, sizeof(T) * Count);
    }

    template<typename T>
    inline void read_as(T* arr, std::size_t count)
    {
        read(arr, sizeof(T) * count);
    }

    template<typename T>
    void read_as_aligned(T& obj, std::size_t alignment = 0)
    {
        align((alignment) ? alignment : alignof(T));
        read(&obj, sizeof(T));
    }

    template<typename T, std::size_t Count>
    void read_as_aligned(T (&arr)[Count], std::size_t alignment = 0)
    {
        align((alignment) ? alignment : alignof(T));
        read(&arr, sizeof(T) * Count);
    }

    template<typename T>
    void read_as_aligned(T* arr, std::size_t count, std::size_t alignment = 0)
    {
        align((alignment) ? alignment : alignof(T));
        read(arr, sizeof(T) * count);
    }

    template<typename T>
    inline void write_as(const T& obj)
    {
        write(&obj, sizeof(T));
    }

    template<typename T, std::size_t Count>
    inline void write_as(const T (&arr)[Count])
    {
        write(&arr, sizeof(T) * Count);
    }

    template<typename T>
    inline void write_as(const T* arr, std::size_t count)
    {
        write(arr, sizeof(T) * count);
    }

    template<typename T>
    void write_as_aligned(const T& obj, std::size_t alignment = 0)
    {
        pad((alignment) ? alignment : alignof(T));
        write(&obj, sizeof(T));
    }

    template<typename T, std::size_t Count>
    void write_as_aligned(const T (&arr)[Count], std::size_t alignment = 0)
    {
        pad((alignment) ? alignment : alignof(T));
        write(&arr, sizeof(T) * Count);
    }

    template<typename T>
    void write_as_aligned(const T* arr, std::size_t count, std::size_t alignment = 0)
    {
        pad((alignment) ? alignment : alignof(T));
        write(arr, sizeof(T) * count);
    }
};
}

#endif

