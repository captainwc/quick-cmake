add_executable(sk_test_printer sk_test_printer.cpp)
target_include_directories(sk_test_printer PUBLIC ${INCLUDE_DIR})
add_test(NAME SK_TEST_PRINTER COMMAND sk_test_printer)

add_executable(sk_test_randomutils sk_test_randomutils.cpp)
target_include_directories(sk_test_randomutils PUBLIC ${INCLUDE_DIR})
add_test(NAME SK_TEST_RANDOM_UTILITIES COMMAND sk_test_randomutils)

add_executable(sk_test_argparser sk_test_argparser.cpp)
target_include_directories(sk_test_argparser PUBLIC ${INCLUDE_DIR})
add_test(NAME SK_TEST_ARGPARSER COMMAND sk_test_argparser)

add_executable(sk_test_timeuitls sk_test_timeutils.cpp)
target_include_directories(sk_test_timeuitls PUBLIC ${INCLUDE_DIR})

if(GTest_FOUND)
    add_executable(sk_test_string_utils sk_test_stringutils.cpp)
    target_include_directories(sk_test_string_utils PUBLIC ${INCLUDE_DIR})
    target_link_libraries(sk_test_string_utils GTest::gtest_main)
    add_test(NAME SK_TEST_STRING_UTILITIES COMMAND sk_test_string_utils)
endif()

if(benchmark_FOUND AND spdlog_FOUND)
    add_executable(sk_benchmark_threadpool sk_benchmark_threadpool.cpp)
    target_include_directories(sk_benchmark_threadpool PUBLIC ${INCLUDE_DIR})
    target_link_libraries(sk_benchmark_threadpool benchmark::benchmark spdlog::spdlog)
endif()

add_subdirectory(containers)
