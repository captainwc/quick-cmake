add_executable(demo_coroutine demo_coroutine.cpp)

add_executable(demo_filesystem demo_filesystem.cpp)
target_include_directories(demo_filesystem PUBLIC ${INCLUDE_DIR})

add_executable(demo_chrono demo_chrono.cpp)
target_include_directories(demo_chrono PUBLIC ${INCLUDE_DIR})

if(benchmark_FOUND AND TBB_FOUND)
    add_executable(bm_paral_algo bm_paral_algo.cpp)
    target_include_directories(bm_paral_algo PRIVATE ${INCLUDE_DIR})
    # the paral algorithms in CXX17 should link library tbb, minimum needed when use gcc.
    target_link_libraries(bm_paral_algo TBB::tbb benchmark::benchmark)
endif()
