add_executable(demo_benchmark demo_benchmark.cpp)
add_executable(demo_gtest demo_gtest.cpp)
add_executable(demo_spdlog demo_spdlog.cpp)

target_link_libraries(demo_benchmark benchmark::benchmark)
target_link_libraries(demo_gtest GTest::gtest_main)
target_link_libraries(demo_spdlog spdlog::spdlog)
