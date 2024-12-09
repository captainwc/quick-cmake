add_executable(sk_test_printer sk_test_printer.cpp)
target_include_directories(sk_test_printer PUBLIC ${INCLUDE_DIR})

if(GTest_FOUND)
    add_executable(sk_test_stringutilities sk_test_stringutilities.cpp)
    target_include_directories(sk_test_stringutilities PUBLIC ${INCLUDE_DIR})
    target_link_libraries(sk_test_stringutilities GTest::gtest_main)

    add_test(NAME SK_TEST_STRING_UTILITIES COMMAND sk_test_stringutilities)
endif()

if(benchmark_FOUND AND spdlog_FOUND)
    add_executable(sk_benchmark_threadpool sk_benchmark_threadpool.cpp)
    target_include_directories(sk_benchmark_threadpool PUBLIC ${INCLUDE_DIR})
    target_link_libraries(sk_benchmark_threadpool benchmark::benchmark spdlog::spdlog)
endif()

add_test(NAME SK_TEST_PRINTER COMMAND sk_test_printer)
