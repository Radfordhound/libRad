/// @file rad_endian.h
/// @author Graham Scott
/// @brief TODO
/// @date 2024-11-14
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_ENDIAN_H_INCLUDED
#define RAD_ENDIAN_H_INCLUDED

#include <climits>

#ifdef _MSC_VER
    #include <stdlib.h>
    #include <type_traits>
#endif

// Target endianness preprocessor defines
#ifndef RAD_TARGET_IS_BIG_ENDIAN
    #ifdef __BYTE_ORDER__ // GCC/Clang

        #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            #define RAD_TARGET_IS_BIG_ENDIAN 1
        #else
            #define RAD_TARGET_IS_BIG_ENDIAN 0
        #endif

    #else // Other (assume little-endian)
        #define RAD_TARGET_IS_BIG_ENDIAN 0
    #endif
#endif

#ifndef RAD_TARGET_IS_LITTLE_ENDIAN
    #ifdef __BYTE_ORDER__ // GCC/Clang

        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            #define RAD_TARGET_IS_LITTLE_ENDIAN 1
        #else
            #define RAD_TARGET_IS_LITTLE_ENDIAN 0
        #endif

    #else // Other (assume little-endian)
        #define RAD_TARGET_IS_LITTLE_ENDIAN 1
    #endif
#endif

// Endian-swap intrinsic macros
namespace rad::endian
{
namespace detail_
{
    constexpr unsigned short byteswap_u16(unsigned short val) noexcept
    {
        return (
            ((val & 0xFFU) << 8) |
            ((val & 0xFF00U) >> 8)
        );
    }

    constexpr unsigned long byteswap_u32(unsigned long val) noexcept
    {
        return (
            ((val & 0xFFU) << 24) |
            ((val & 0xFF00U) << 8) |
            ((val & 0xFF0000U) >> 8) |
            ((val & 0xFF000000U) >> 24)
        );
    }

    constexpr unsigned long long byteswap_u64(unsigned long long val) noexcept
    {
        return (
            ((val & 0xFFU) << 56) |
            ((val & 0xFF00U) << 40) |
            ((val & 0xFF0000U) << 24) |
            ((val & 0xFF000000U) << 8) |
            ((val & 0xFF00000000U) >> 8) |
            ((val & 0xFF0000000000U) >> 24) |
            ((val & 0xFF000000000000U) >> 40) |
            ((val & 0xFF00000000000000U) >> 56)
        );
    }
}

#ifdef _MSC_VER // MSVC intrinsics

    #define RAD_BUILTIN_SWAP_U16(v) _byteswap_ushort(v)
    #define RAD_BUILTIN_SWAP_U32(v) _byteswap_ulong(v)
    #define RAD_BUILTIN_SWAP_U64(v) _byteswap_uint64(v)

#elif defined(__clang__) || defined(__GNUC__) // Clang/GCC intrinsics

    #define RAD_BUILTIN_SWAP_U16(v) __builtin_bswap16(v)
    #define RAD_BUILTIN_SWAP_U32(v) __builtin_bswap32(v)
    #define RAD_BUILTIN_SWAP_U64(v) __builtin_bswap64(v)

#endif

// Fallback macros
#ifndef RAD_BUILTIN_SWAP_U16
    #define RAD_BUILTIN_SWAP_U16(v) ::rad::endian::detail_::byteswap_u16(v)
#endif

#ifndef RAD_BUILTIN_SWAP_U32
    #define RAD_BUILTIN_SWAP_U32(v) ::rad::endian::detail_::byteswap_u32(v)
#endif

#ifndef RAD_BUILTIN_SWAP_U64
    #define RAD_BUILTIN_SWAP_U64(v) ::rad::endian::detail_::byteswap_u64(v)
#endif

// Endian-swap constexpr functions.

// NOTE: The intrinsic macros are still needed and preferred because,
// disappointingly, MSVC isn't smart and often doesn't optimize
// these properly (i.e. replace them with bswap instructions).

constexpr unsigned short constexpr_byteswap_u16(unsigned short val) noexcept
{
    return (
#if defined(__clang__) || defined(__GNUC__)
        __builtin_bswap16(val)
#else
    #if defined(_MSC_VER) && defined(__cpp_lib_is_constant_evaluated)
        (!std::is_constant_evaluated()) ? _byteswap_ushort(val) :
    #endif
        detail_::byteswap_u16(val)
#endif
    );
}

constexpr unsigned long constexpr_byteswap_u32(unsigned long val) noexcept
{
    return (
#if defined(__clang__) || defined(__GNUC__)
        __builtin_bswap32(val)
#else
    #if defined(_MSC_VER) && defined(__cpp_lib_is_constant_evaluated)
        (!std::is_constant_evaluated()) ? _byteswap_ulong(val) :
    #endif
        detail_::byteswap_u32(val)
#endif
    );
}

constexpr unsigned long long constexpr_byteswap_u64(unsigned long long val) noexcept
{
    return (
#if defined(__clang__) || defined(__GNUC__)
        __builtin_bswap64(val)
#else
    #if defined(_MSC_VER) && defined(__cpp_lib_is_constant_evaluated)
        (!std::is_constant_evaluated()) ? _byteswap_uint64(val) :
    #endif
        detail_::byteswap_u64(val)
#endif
    );
}

template<typename T>
constexpr T constexpr_byteswap(T val) noexcept
{
    static_assert((
        sizeof(T) * CHAR_BIT == 8 ||
        sizeof(T) * CHAR_BIT == 16 ||
        sizeof(T) * CHAR_BIT == 32 ||
        sizeof(T) * CHAR_BIT == 64),
        "Unsupported integer bit-width"
    );

    // TODO: Handle floating points.

    if constexpr (sizeof(T) * CHAR_BIT == 16) // 16-bit
    {
        return static_cast<T>(
            constexpr_byteswap_u16(static_cast<unsigned short>(val))
        );
    }
    else if constexpr (sizeof(T) * CHAR_BIT == 32) // 32-bit
    {
        return static_cast<T>(
            constexpr_byteswap_u32(static_cast<unsigned long>(val))
        );
    }
    else if constexpr (sizeof(T) * CHAR_BIT == 64) // 64-bit
    {
        return static_cast<T>(
            constexpr_byteswap_u64(static_cast<unsigned long long>(val))
        );
    }
    else // 8-bit
    {
        return val;
    }
}

template<typename T>
inline T fast_byteswap(T val) noexcept
{
    static_assert((
        sizeof(T) * CHAR_BIT == 8 ||
        sizeof(T) * CHAR_BIT == 16 ||
        sizeof(T) * CHAR_BIT == 32 ||
        sizeof(T) * CHAR_BIT == 64),
        "Unsupported integer bit-width"
    );

    static_assert(
        std::is_integral_v<T> || std::is_floating_point_v<T>,
        "Can only byteswap numeric values"
    );

    if constexpr (std::is_floating_point_v<T>)
    {
        // TODO: Use std::bit_cast instead of union type punning !!!

        if constexpr (sizeof(T) * CHAR_BIT == 16) // 16-bit
        {
            union
            {
                unsigned short integral;
                T floating;
            }
            swappedVal;

            swappedVal.floating = val;
            swappedVal.integral = RAD_BUILTIN_SWAP_U16(swappedVal.integral);

            return swappedVal.floating;
        }
        else if constexpr (sizeof(T) * CHAR_BIT == 32) // 32-bit
        {
            union
            {
                unsigned long integral;
                T floating;
            }
            swappedVal;

            swappedVal.floating = val;
            swappedVal.integral = RAD_BUILTIN_SWAP_U32(swappedVal.integral);

            return swappedVal.floating;
        }
        else if constexpr (sizeof(T) * CHAR_BIT == 64) // 64-bit
        {
            union
            {
                unsigned long long integral;
                T floating;
            }
            swappedVal;

            swappedVal.floating = val;
            swappedVal.integral = RAD_BUILTIN_SWAP_U64(swappedVal.integral);

            return swappedVal.floating;
        }
        else // 8-bit
        {
            return val;
        }
    }
    else
    {
        if constexpr (sizeof(T) * CHAR_BIT == 16) // 16-bit
        {
            return static_cast<T>(
                RAD_BUILTIN_SWAP_U16(static_cast<unsigned short>(val))
            );
        }
        else if constexpr (sizeof(T) * CHAR_BIT == 32) // 32-bit
        {
            return static_cast<T>(
                RAD_BUILTIN_SWAP_U32(static_cast<unsigned long>(val))
            );
        }
        else if constexpr (sizeof(T) * CHAR_BIT == 64) // 64-bit
        {
            return static_cast<T>(
                RAD_BUILTIN_SWAP_U64(static_cast<unsigned long long>(val))
            );
        }
        else // 8-bit
        {
            return val;
        }
    }
}

template<typename T>
inline T little_to_native(T val) noexcept
{
#if RAD_TARGET_IS_LITTLE_ENDIAN
    return val;
#else
    return fast_byteswap(val);
#endif
}

template<typename T>
inline T big_to_native(T val) noexcept
{
#if RAD_TARGET_IS_BIG_ENDIAN
    return val;
#else
    return fast_byteswap(val);
#endif
}

template<typename T>
inline T native_to_little(T val) noexcept
{
#if RAD_TARGET_IS_LITTLE_ENDIAN
    return val;
#else
    return fast_byteswap(val);
#endif
}

template<typename T>
inline T native_to_big(T val) noexcept
{
#if RAD_TARGET_IS_BIG_ENDIAN
    return val;
#else
    return fast_byteswap(val);
#endif
}
}

#endif
