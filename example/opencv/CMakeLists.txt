macro(AddOpenCVExecutable source)
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target} ${source})
    target_include_directories(${target} PRIVATE ${INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(${target} ${OpenCV_LIBS})
    target_compile_features(${target} PRIVATE cxx_std_20)
endmacro()

if(OpenCV_FOUND)
    AddOpenCVExecutable(demo_opencv.cpp)
    AddOpenCVExecutable(demo_opencv_car.cpp)
    AddOpenCVExecutable(demo_opencv_coro.cpp)
endif()
