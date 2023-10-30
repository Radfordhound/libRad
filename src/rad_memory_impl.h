/**
 * @file rad_memory_impl.h
 * @author Graham Scott
 * @brief Helper header file to be used by implementations of memory utilities.
 * @version 0.1
 * @date 2023-04-07
 * 
 * @copyright Copyright (c) 2023 Graham Scott
 */
#ifndef RAD_MEMORY_IMPL_H_INCLUDED
#define RAD_MEMORY_IMPL_H_INCLUDED

#define RAD_VALIDATE_ALIGNED_ALLOC_ARGS(size, alignment)\
{\
    assert(((alignment) & ((alignment) - 1)) == 0 &&\
        "The given alignment must be a power of 2");\
\
    assert(((alignment) % sizeof(void*)) != 0 &&\
        "The given alignment must be a multiple of sizeof(void*)");\
}

#endif
