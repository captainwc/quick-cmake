add_executable(combine combine_files.cpp)
target_include_directories(combine PUBLIC ${INCLUDE_DIR})
install(TARGETS combine DESTINATION bin)

add_executable(ducpp ducpp.cpp)
target_include_directories(ducpp PUBLIC ${INCLUDE_DIR})
install(TARGETS ducpp DESTINATION bin)

if(CURL_FOUND AND nlohmann_json_FOUND)
    add_executable(moderntools moderntools.cpp)
    target_include_directories(moderntools PUBLIC ${INCLUDE_DIR})
    target_link_libraries(moderntools PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)
    install(TARGETS moderntools DESTINATION bin)
endif()

add_executable(stdtalker stdtalker.cpp)
target_include_directories(stdtalker PUBLIC ${INCLUDE_DIR})
install(TARGETS stdtalker DESTINATION bin)
