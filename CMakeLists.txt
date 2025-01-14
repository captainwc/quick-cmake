cmake_minimum_required(VERSION 3.16)
project(utils)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "Do not build in the source directory. Please create a separate build directory.")
endif()

macro(set_git_home)
    find_program(GIT_EXECUTABLE git REQUIRED)
    get_filename_component(GIT_BIN_DIR ${GIT_EXECUTABLE} DIRECTORY)
    get_filename_component(GIT_ROOT ${GIT_BIN_DIR} DIRECTORY)
    message(STATUS "GIT ROOT: ${GIT_ROOT}")
endmacro()
    
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(SYSTEM "win")
    set_git_home()
    set(CMAKE_PREFIX_PATH "${GIT_ROOT}/opt/vcpkg/installed/x64-mingw-static/")
else()
    set(SYSTEM "unix")
endif()

include(cmake/settings.cmake)
include(cmake/cpack.cmake)
