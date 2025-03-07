# 基本设置
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 基本路径设置
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# 库文件查找路径
list(APPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/$ENV{VCPKG_DEFAULT_TRIPLET}")
message(STATUS ${CMAKE_PREFIX_PATH})

# 安装位置
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Default install path")
endif()

# 安装头文件
install(DIRECTORY ${INCLUDE_DIR}/skutils DESTINATION include)

include(${CMAKE_SOURCE_DIR}/cmake/tools.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/library.cmake)

# CXX相关设置
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

option(WERROR "if -Werror" OFF)
if(WERROR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

option(BUILD_WITH_COVERAGE "Enable coverage reporting using gcov" OFF)
if(BUILD_WITH_COVERAGE)
    include(${CMAKE_SOURCE_DIR}/cmake/coverage.cmake)
endif()

enable_testing()

add_subdirectory(src)
add_subdirectory(test)

option(BUILD_EXAMPLE "whether contains some lib demo" ON)

if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
