# set(LIBS_ROOT_PATH "D:/env/libs" CACHE PATH "Additional path to search for external libraries")

# set(CMAKE_PREFIX_PATH "${LIBS_ROOT_PATH};${CMAKE_PREFIX_PATH}")

# find_package(GTest)
# find_package(benchmark)
# find_package(spdlog)
# find_package(Boost COMPONENTS system)
# find_package(OpenCV)
# # find_package(rclcpp)
# find_package(Protobuf)

local_find_package(GTest)
local_find_package(benchmark)
local_find_package(spdlog)
local_find_package(Boost COMPONENTS system)
local_find_package(OpenCV)
local_find_package(Protobuf)
local_find_package(TBB)
local_find_package(CURL)
local_find_package(nlohmann_json)
local_find_package(tl-expected)
local_find_package(cpptrace)


# if you dont have dbg.h, uncomment belows:

# dbg-macro
# include(FetchContent)
# FetchContent_Declare(dbg_macro GIT_REPOSITORY https://github.com/sharkdp/dbg-macro)
# FetchContent_MakeAvailable(dbg_macro)
