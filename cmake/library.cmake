set(LIBS_ROOT_PATH "D:/env/libs" CACHE PATH "Additional path to search for external libraries")

set(CMAKE_PREFIX_PATH "${LIBS_ROOT_PATH};${CMAKE_PREFIX_PATH}")

find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)
find_package(spdlog REQUIRED)

