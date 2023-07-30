/**
 * @file rad_base.h
 * @author Graham Scott
 * @brief Header file providing base macros required by most header files in libRad.
 * @version 0.1
 * @date 2023-04-07
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 * 
 */
#ifndef RAD_BASE_H_INCLUDED
#define RAD_BASE_H_INCLUDED

// Shared library macros
// (Adapted from https://gcc.gnu.org/wiki/Visibility)
#ifndef RAD_API
    #ifdef RAD_IS_DLL
        #ifdef _WIN32
            #ifdef RAD_IS_BUILDING_DLL
                // We're building a Windows DLL; export the given symbol.
                #define RAD_API __declspec(dllexport)
            #else
                // We're using a pre-built Windows DLL; import the given symbol.
                #define RAD_API __declspec(dllimport)
            #endif
        #elif defined(__GNUC__) && __GNUC__ >= 4 
            // We're using GCC (or compatible); use __attribute__
            #define RAD_API __attribute__ ((visibility ("default")))
        #else
            // We don't know the target; just assume it requires no keywords.
            #define RAD_API
        #endif
    #else
        // This is a static library; no keyword(s) are needed.
        #define RAD_API
    #endif
#endif

// Debug memory allocation
// NOTE: If this value is not set to 1 while compiling libRad,
// debug memory allocation functions will not be available!
#ifndef RAD_USE_DEBUG_MEMORY
    #ifndef NDEBUG
        #define RAD_USE_DEBUG_MEMORY 1
    #else
        #define RAD_USE_DEBUG_MEMORY 0
    #endif
#endif

// Operator new/delete replacements
#ifndef RAD_USE_OPERATOR_NEW_DELETE_REPLACEMENTS
    // NOTE: If this value is not defined while compiling libRad,
    // it will fallback to its default value of 1!
    #define RAD_USE_OPERATOR_NEW_DELETE_REPLACEMENTS 1
#endif

#endif
