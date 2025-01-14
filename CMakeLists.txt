cmake_minimum_required(VERSION 3.16)
project(utils)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "Do not build in the source directory. Please create a separate build directory.")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(SYSTEM "win")
    set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/$ENV{VCPKG_DEFAULT_TRIPLET}/")
else()
    set(SYSTEM "unix")
endif()

include(cmake/settings.cmake)
include(cmake/cpack.cmake)
