add_executable(combine combine_files.cpp)
target_include_directories(combine PUBLIC ${INCLUDE_DIR})

install(TARGETS combine DESTINATION bin)
