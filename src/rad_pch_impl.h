/// @file rad_pch_impl.h
/// @author Graham Scott
/// @brief Precompiled header file for libRad.
/// @version 0.1
/// @date 2023-03-31
/// @copyright Copyright (c) Graham Scott; see LICENSE.txt file for details
/// 
/// This file is automatically included in every
/// source file in the project thanks to CMake.

#ifndef RAD_PCH_IMPL_H_INCLUDED
#define RAD_PCH_IMPL_H_INCLUDED

// POSIX defines
#ifndef _WIN32
    #define _DARWIN_USE_64_BIT_INODE 1
    #define _FILE_OFFSET_BITS 64
    #define _LARGEFILE64_SOURCE 1
#endif

// Standard library includes
#include <new>
#include <stdexcept>
#include <system_error>
#include <cstring>
#include <cassert>
#include <cerrno>

// Windows platform includes
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#endif

#endif
