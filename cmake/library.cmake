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

