# 基本设置
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 基本路径设置
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

include(${CMAKE_SOURCE_DIR}/cmake/library.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/tools.cmake)

# CXX相关设置
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

option(WERROR "if -Werror" OFF)
if (WERROR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif ()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(benchmark)

option(BUILD_EXAMPLE "whether contains some lib demo" ON)
if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
