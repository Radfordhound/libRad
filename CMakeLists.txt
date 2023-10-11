# Setup CMake
cmake_minimum_required(VERSION 3.18...3.26)

if(WIN32)
    set(CMAKE_DEBUG_POSTFIX d)
    set(RAD_USE_DEBUG_POSTFIX
        DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    )
endif()

# Setup project
project(libRad
    VERSION 1.0.0
    LANGUAGES CXX
)

# Include dependencies
#include(FetchContent)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Options
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    set(RAD_ROOT_CMAKE_FILE ON)
else()
    set(RAD_ROOT_CMAKE_FILE OFF)
endif()

option(BUILD_SHARED_LIBS
    "Build libRad as a shared library instead of a static library"
    OFF
)

# Platform-specific options
if(WIN32)
    option(RAD_WIN32_FORCE_ANSI
        "Force libRad to use the ANSI Win32 API instead of wide-char UNICODE Win32 functions (not recommended)"
        OFF
    )
endif()

# Make CMake output all libraries and runtimes to top-level binary directory
# (This is so .dlls will be in same folder as .exes on Windows if building shared libraries)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set directories
set(RAD_INCLUDE_DIR "include/rad")
set(RAD_SOURCE_DIR "src")

# Set includes
set(RAD_INCLUDES
    "${RAD_INCLUDE_DIR}/rad_allocator_traits.h"
    "${RAD_INCLUDE_DIR}/rad_base.h"
    "${RAD_INCLUDE_DIR}/rad_default_allocator.h"
    "${RAD_INCLUDE_DIR}/rad_defer.h"
    "${RAD_INCLUDE_DIR}/rad_memory_pool.h"
    "${RAD_INCLUDE_DIR}/rad_memory.h"
    "${RAD_INCLUDE_DIR}/rad_object_utils.h"
    "${RAD_INCLUDE_DIR}/rad_pair.h"
    "${RAD_INCLUDE_DIR}/rad_scoped_enum_helpers.h"
    "${RAD_INCLUDE_DIR}/rad_stack_or_heap_array.h"
    "${RAD_INCLUDE_DIR}/rad_stack_or_heap_memory.h"
    "${RAD_INCLUDE_DIR}/rad_vector.h"
)

# Set sources
set(RAD_SOURCES
    "${RAD_SOURCE_DIR}/rad_memory_impl.cpp"
    "${RAD_SOURCE_DIR}/rad_memory_impl.h"
    "${RAD_SOURCE_DIR}/rad_pch_impl.h"
)

set(RAD_PCH_PATH "${RAD_SOURCE_DIR}/rad_pch_impl.h")

# Add platform-specific sources
if(WIN32)
    list(APPEND RAD_SOURCES
        "${RAD_SOURCE_DIR}/rad_memory_win32.cpp"
    )
else()
    list(APPEND RAD_SOURCES
        "${RAD_SOURCE_DIR}/rad_memory_posix.cpp"
    )
endif()

# Setup library
add_library(libRad ${RAD_INCLUDES} ${RAD_SOURCES})
add_library(libRad::libRad ALIAS libRad)

set_target_properties(libRad PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    ${RAD_USE_DEBUG_POSTFIX}
)

target_precompile_headers(libRad
    PRIVATE ${RAD_PCH_PATH}
 )

# Setup DLL preprocessor definitions
if(BUILD_SHARED_LIBS)
    target_compile_definitions(libRad
        PUBLIC RAD_IS_DLL
        PRIVATE RAD_IS_BUILDING_DLL
    )
endif()

# Setup platform-specific settings
if(WIN32)
    if(NOT RAD_WIN32_FORCE_ANSI)
        target_compile_definitions(libRad
            PRIVATE
                _UNICODE
                UNICODE
        )
    endif()
elseif(CMAKE_SHARED_LIBRARY_PREFIX STREQUAL "lib")
    # Ensures the output name is "libRad", not "liblibRad"
    set_target_properties(libRad PROPERTIES OUTPUT_NAME "Rad")
endif()

# Compiler-specific settings
if(MSVC)
    # Disable annoying MSVC unsafe string warnings
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

    # Enable strict conformance mode in MSVC
    add_compile_options(/permissive-)
endif()


# Setup include directories
target_include_directories(libRad
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${RAD_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Install include files
install(DIRECTORY ${RAD_INCLUDE_DIR}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Setup CMake package config files
install(TARGETS libRad
    EXPORT libRadTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(RAD_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/libRad)

install(EXPORT libRadTargets
    FILE libRadTargets.cmake
    NAMESPACE libRad::
    DESTINATION ${RAD_INSTALL_CONFIGDIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/libRadConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/libRadConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libRadConfig.cmake
    INSTALL_DESTINATION ${RAD_INSTALL_CONFIGDIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libRadConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libRadConfigVersion.cmake
    DESTINATION ${RAD_INSTALL_CONFIGDIR}
)

# Install license file
install(FILES
    ${PROJECT_SOURCE_DIR}/LICENSE.txt
    DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/libRad
)

# Export CMake library
export(EXPORT libRadTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/libRadTargets.cmake
    NAMESPACE libRad::
)

export(PACKAGE libRad)
