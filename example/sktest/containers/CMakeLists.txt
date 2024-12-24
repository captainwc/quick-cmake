add_executable(sk_test_graph sk_test_graph.cpp)
target_include_directories(sk_test_graph PUBLIC ${INCLUDE_DIR})

add_executable(sk_test_topkqueue sk_test_topkqueue.cpp)
target_include_directories(sk_test_topkqueue PUBLIC ${INCLUDE_DIR})
add_test(NAME SK_TEST_TOP_BOTTOM_K_QUEUE COMMAND sk_test_topkqueue )
