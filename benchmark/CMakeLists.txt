add_executable(BM_ThreadPool BM_ThreadPool.cpp)

target_include_directories(BM_ThreadPool PRIVATE ${INCLUDE_DIR})
target_link_libraries(BM_ThreadPool benchmark::benchmark)
target_link_libraries(BM_ThreadPool spdlog::spdlog)