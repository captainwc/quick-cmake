add_executable(demo_coro boost/demo_coro.cpp)

if(benchmark_FOUND)
    add_executable(demo_benchmark google/demo_benchmark.cpp)
    target_link_libraries(demo_benchmark benchmark::benchmark)
endif()

if(GTest_FOUND)
    add_executable(demo_gtest google/demo_gtest.cpp)
    target_link_libraries(demo_gtest GTest::gtest_main)
endif()

if(spdlog_FOUND)
    add_executable(demo_spdlog logger/demo_spdlog.cpp)
    target_link_libraries(demo_spdlog spdlog::spdlog)
endif()

if(Boost_FOUND)
    add_executable(demo_asio_server boost/demo_asio_server.cpp)
    add_executable(demo_asio_client boost/demo_asio_client.cpp)
    if(${SYSTEM} STREQUAL "win")
        target_link_libraries(demo_asio_server Boost::system ws2_32)
        target_link_libraries(demo_asio_client Boost::system ws2_32)
    else()
        target_link_libraries(demo_asio_server Boost::system)
        target_link_libraries(demo_asio_client Boost::system)
    endif()
endif()

if(rclcpp_FOUND)

endif()

if(OpenCV_FOUND)
    add_executable(demo_opencv opencv/demo_opencv.cpp)
    target_include_directories(demo_opencv PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(demo_opencv ${OpenCV_LIBS})
endif()

add_test(NAME GTEST_DEMO COMMAND demo_gtest)
