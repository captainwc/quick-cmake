# set(LIBS_ROOT_PATH "D:/env/libs" CACHE PATH "Additional path to search for external libraries")

# set(CMAKE_PREFIX_PATH "${LIBS_ROOT_PATH};${CMAKE_PREFIX_PATH}")

find_package(GTest)
find_package(benchmark)
find_package(spdlog)
find_package(Boost COMPONENTS system)
find_package(OpenCV)
find_package(rclcpp)
