add_executable(demo_coroutine demo_coroutine.cpp)

if(benchmark_FOUND)
    add_executable(bm_paral_algo bm_paral_algo.cpp)
    target_include_directories(bm_paral_algo PRIVATE ${INCLUDE_DIR})
    # the paral algorithms in CXX17 should link library tbb, minimum needed when use gcc.
    target_link_libraries(bm_paral_algo tbb benchmark::benchmark)
endif()
