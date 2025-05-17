macro(AddOpenCVExecutable source)
# cmake_path(GET source FILENAME target)
get_filename_component(target ${source} NAME_WE)
add_executable(${target} ${source})
target_include_directories(${target} PRIVATE ${INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${target} ${OpenCV_LIBS})

add_custom_command(TARGET ${target}
POST_BUILD
COMMAND cp ${OpenCV_DIR}/../bin/* ${EXECUTABLE_OUTPUT_PATH}/)
endmacro()


if(OpenCV_FOUND)
AddOpenCVExecutable(demo_opencv.cpp)
AddOpenCVExecutable(demo_opencv_car.cpp)
endif()
