if(Boost_FOUND)
    add_executable(demo_asio_server demo_asio_server.cpp)
    add_executable(demo_asio_client demo_asio_client.cpp)
    if(${SYSTEM} STREQUAL "win")
        target_link_libraries(demo_asio_server Boost::system ws2_32)
        target_link_libraries(demo_asio_client Boost::system ws2_32)
    else()
        target_link_libraries(demo_asio_server Boost::system)
        target_link_libraries(demo_asio_client Boost::system)
    endif()

    add_executable(demo_boost_graph demo_boost_graph.cpp)
    target_link_libraries(demo_boost_graph Boost::system)
endif()