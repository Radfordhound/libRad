/**
 * @file rad_pch_impl.h
 * @author Graham Scott
 * @brief Precompiled header file for libRad.
 * @version 0.1
 * @date 2023-03-31
 * @copyright Copyright (c) 2023 Graham Scott
 * 
 * This file is automatically included in every
 * source file in the project thanks to CMake.
 */
#ifndef RAD_PCH_IMPL_H_INCLUDED
#define RAD_PCH_IMPL_H_INCLUDED

// Standard library includes
#include <cassert>

// Windows platform includes
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#endif

#endif
