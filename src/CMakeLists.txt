add_executable(combine combine_files.cpp)
target_include_directories(combine PUBLIC ${INCLUDE_DIR})
install(TARGETS combine DESTINATION bin)

add_executable(ducpp ducpp.cpp)
target_include_directories(ducpp PUBLIC ${INCLUDE_DIR})
install(TARGETS ducpp DESTINATION bin)
