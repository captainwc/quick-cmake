add_executable(demo_benchmark demo_benchmark.cpp)
target_link_libraries(demo_benchmark benchmark::benchmark)

add_executable(demo_gtest demo_gtest.cpp)
target_link_libraries(demo_gtest GTest::gtest_main)

add_executable(demo_spdlog demo_spdlog.cpp)
target_link_libraries(demo_spdlog spdlog::spdlog)


# asio: 实测，c++20会报错没有std::exchange，改用c++17可以通过编译；另外 windows 上要链接上 ws2_32 库，防止链接不到 winsock 啥的
add_executable(demo_asio_server demo_asio_server.cpp)
add_executable(demo_asio_client demo_asio_client.cpp)
if(${SYSTEM} STREQUAL "win")
    target_link_libraries(demo_asio_server Boost::system ws2_32)
    target_link_libraries(demo_asio_client Boost::system ws2_32)
else()
    target_link_libraries(demo_asio_server Boost::system)
    target_link_libraries(demo_asio_client Boost::system)
endif()
