/// @file rad_file.h
/// @author Graham Scott
/// @brief TODO
/// @date 2024-07-05
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_FILE_H_INCLUDED
#define RAD_FILE_H_INCLUDED

#include "rad_stream.h"
#include "rad_vector.h"
#include <vector>
#include <cstdint>
#include <cstddef>

namespace rad
{
class file_stream
    : public stream
{
    std::uintmax_t  handle_;

public:
    enum open_flags : unsigned long
    {
        OPEN_MASK_MODE = 0xF,
        OPEN_MASK_HINT = 0xF0000,

        OPEN_MODE_READ_ONLY = 0x0,
        OPEN_MODE_WRITE_ONLY = 0x1,
        OPEN_MODE_READ_WRITE = 0x2,
        //OPEN_MODE_EXECUTE = 0x3,


        /// @brief Share the file with other processes.
        OPEN_FLAG_SHARED = 0x10,

        /// @brief Create the file if it doesn't exist, or
        /// truncate ("re-create") the file if it does exist.
        OPEN_FLAG_CREATE = 0x20,


        /// @brief Don't optimize the file for any particular
        /// type of access; instead, let the operating system
        /// use its default optimization strategies.
        OPEN_HINT_NORMAL_ACCESS = 0x0,

        /// @brief Optimize the file for sequential access -
        /// that is, reading/writing the file in order, from
        /// beginning to end.
        OPEN_HINT_SEQUENTIAL_ACCESS = 0x10000,

        /// @brief Optimize the file for random access -
        /// that is, reading/writing the file in "random"
        /// order, in no particular pattern.
        OPEN_HINT_RANDOM_ACCESS = 0x20000,
    };

private:
    [[nodiscard]] static std::uintmax_t try_open_(
        const char* filePath,
        unsigned long flags
    );

    static bool try_close_(std::uintmax_t handle) noexcept;

    std::size_t try_read_chunk_(void* buf, std::size_t size);

    std::size_t try_write_chunk_(const void* buf, std::size_t size);

public:
    /// @brief Retrieves the total number of bytes currently in the stream.
    /// @return The total number of bytes currently in the stream.
    RAD_API unsigned long long get_size() const override;

    /// @brief Flushes any pending writes made to the stream, if necessary.
    RAD_API void flush() override;

    RAD_API std::size_t try_read(void* buf, std::size_t size) override;

    RAD_API std::size_t try_write(const void* buf, std::size_t size) override;

    RAD_API void seek(seek_mode mode, long long offset) override;

    RAD_API void jump_to(unsigned long long pos) override;

    /// @brief Attempt to (re)-open the stream using the file at the given path.
    ///
    /// @details If read mode is specified and a file does not exist at the given path,
    /// this function will simply return false; the state of the existing stream will
    /// not be modified.
    ///
    /// If the file fails to open for any other reason, this function will throw an exception.
    ///
    /// If write mode is specified and a file does not exist at the given path, this function
    /// will attempt to create the file and open it in write mode, throwing an exception if
    /// this fails.
    ///
    /// If the file is successfully opened, any existing handles will be closed, the state
    /// of the existing stream will be reset (invalidated) as if a new stream has just been
    /// created in its place, and true will be returned.
    ///
    /// This function provides a strong exception guarantee. If any exception is thrown, the
    /// state of the existing stream will not be modified.
    ///
    /// @param filePath The path to the file to open, represented as a UTF-8 string.
    /// Can be relative, include symlinks, etc. On Windows, the path will be internally converted
    /// to UTF-16, and paths greater than MAX_PATH (260 characters) are supported.
    ///
    /// @param flags The flags to be used to open the file.
    /// See `open_flags` documentation for details.
    ///
    /// @return True if the file was opened successfully;
    /// false if a file at the given path did not exist.
    RAD_API bool try_reopen(const char* filePath, unsigned long flags);

    /// @brief Attempt to close the currently-open handle now, if there is one.
    ///
    /// @details If the file_stream does not have a currently-open handle,
    /// this function will simply return true.
    ///
    /// If the file_stream does have a currently-open handle, this function will
    /// attempt to close it, and return true if it succeeded, or false if it failed.
    ///
    /// Regardless of whether it succeeds, it will reset (invalidate) the state of the
    /// existing stream. This is necessary due to the unfortunate fact that attempting to
    /// close a file handle that we have already attempted and failed to close can result
    /// in undefined/unexpected behavior on some operating systems; so there's nothing we
    /// can actually do about it.
    ///
    /// Does NOT necessarily flush any open handle's data; call flush()
    /// if you need this.
    ///
    /// NOTE: Any open handle will automatically be closed by the
    /// file_stream destructor; so you do NOT have to call this function
    /// unless you wish to close the file early, or know if it successfully
    /// closed or not.
    ///
    /// @return True if an existing open handle was successfully closed, or if there
    /// was no existing open handle. False if there was an existing open handle, and
    /// it failed to close.
    RAD_API bool try_close() noexcept;

    /// @brief Close the currently-open handle now, if there is one.
    /// Throws an exception if the handle could not be closed.
    ///
    /// @details If the file_stream does not have a currently-open
    /// handle, this function has no effect.
    ///
    /// Does NOT necessarily flush any open handle's data; call flush()
    /// if you need this.
    /// 
    /// NOTE: Any open handle will automatically be closed by the
    /// file_stream destructor; so you do NOT have to call this function
    /// unless you wish to close the file early, or to have an exception
    /// be thrown if it fails to close (the destructor will NOT throw an 
    /// exception on close failure).
    RAD_API void close();

    RAD_API std::uintmax_t release() noexcept;

    RAD_API file_stream() noexcept;

    RAD_API file_stream(
        std::uintmax_t nativeHandle,
        bool canRead,
        bool canWrite
    ) noexcept;

    RAD_API file_stream(const char* filePath, unsigned long flags);

    /// @brief Destructor for the file_stream.
    ///
    /// @details Automatically closes any open handles.
    ///
    /// Does NOT necessarily flush any open handle's data; call flush()
    /// if you need this.
    ///
    /// If an open handle fails to close, the destructor will NOT
    /// throw any exceptions (thus it will never call std::terminate).
    /// The close failure will simply be silently swallowed, and the
    /// operating system will handle this as it sees fit.
    ///
    /// Unfortunately, there's really not a better way to handle this
    /// due to how many operating systems (don't) handle file close failure.
    ///
    /// If you wish to know if the file close attempt failed or not, please
    /// call .try_close() which returns a bool specifying if the file was closed
    /// successfully or not, or .close() which throws an exception if the file fails
    /// to close.
    RAD_API ~file_stream() override;
};
}

#endif

