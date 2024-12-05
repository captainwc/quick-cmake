if(benchmark_FOUND)
    add_executable(demo_benchmark demo_benchmark.cpp)
    target_link_libraries(demo_benchmark benchmark::benchmark)
endif()

if(GTest_FOUND)
    add_executable(demo_gtest demo_gtest.cpp)
    target_link_libraries(demo_gtest GTest::gtest_main)
endif()
