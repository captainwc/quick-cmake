add_executable(printTest printTest.cpp)
target_include_directories(printTest PUBLIC ${INCLUDE_DIR})

add_test(NAME PRINT_TEST COMMAND printTest)
