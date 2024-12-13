if(Protobuf_FOUND)
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS AddressBook.proto)

    message("PROTO_SRCS = ${PROTO_SRCS}")
    message("PROTO_HDRS = ${PROTO_HDRS}")

    add_executable(demo_protobuf demo_protobuf.cpp ${PROTO_SRCS} ${PROTO_HDRS})
    target_include_directories(demo_protobuf PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${INCLUDE_DIR})
    target_link_libraries(demo_protobuf protobuf::libprotobuf)
endif()
