# 为target执行cppcheck
function(run_cppcheck target)
    if(CPP_CHECK)
        message(STATUS "Running cppcheck on ${target}")
        add_custom_command(
            TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove ${CPP_CHECK_DIR}/${target}.log
            COMMAND
                ${CPP_CHECK} --enable=all --suppress=missingIncludeSystem --std=c++20
                --output-file=${CPP_CHECK_DIR}/${target}.log --force --quiet
                --project=${CMAKE_BINARY_DIR}/compile_commands.json
            COMMENT "Running cppcheck on ${target}")
    endif()
endfunction()

# 普通target添加
function(add_targets GROUP_NAME SOURCES)
    set(TARGETS "[${GROUP_NAME}]: ")

    # 【这里可能还需要打磨：遍历的时候必须要 ARGV1 ARGN，才能保证不漏掉源文件】
    foreach(file IN LISTS ARGV1 ARGN)
        get_filename_component(target ${file} NAME_WE)
        add_executable(${target} ${file})
        target_include_directories(${target} PUBLIC ${INCLUDE_DIR})
        set(TARGETS "${TARGETS} ${target}")
        # 这里在windows下能找到cppcheck，但是会出现奇怪的问题，遗憾
        if(${SYSTEM} STREQUAL "unix")
            run_cppcheck(${target})
        endif()
    endforeach()

    message(STATUS --${TARGETS})
endfunction()
