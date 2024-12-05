if(spdlog_FOUND)
    add_executable(demo_spdlog demo_spdlog.cpp)
    target_link_libraries(demo_spdlog spdlog::spdlog)
endif()
