/// @file rad_bit.h
/// @author Graham Scott
/// @brief TODO
/// @date 2025-03-24
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details

#ifndef RAD_BIT_H_INCLUDED
#define RAD_BIT_H_INCLUDED

#include <climits>

namespace rad
{
template<typename T>
constexpr T high_bit() noexcept
{
    return T{1} << ((sizeof(T) * CHAR_BIT) - 1);
}
}

#endif
