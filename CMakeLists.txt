cmake_minimum_required(VERSION 3.16)
project(utils)

set(CMAKE_CXX_STANDARD 20)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(SYSTEM "win")
else()
    set(SYSTEM "unix")
endif()

include(cmake/settings.cmake)
