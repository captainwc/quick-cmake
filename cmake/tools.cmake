# ANSI COLOR
set(FG_RED \\033[0;31m)
set(FG_GREEN \\033[0;32m)
set(FG_YELLOW \\033[0;33m)
set(FG_BLUE \\033[0;36m)
set(BG_RED \\033[41m)
set(BG_GREEN \\033[42m)
set(BG_YELLOW \\033[43m)
set(BG_BLUE \\033[44m)
set(COLOR_RESET \\033[0m)
# message red
function(msg_red)
    execute_process(COMMAND echo -en ${FG_RED}${ARGN}\n${COLOR_RESET})
endfunction()
# message green
function(msg_green)
    execute_process(COMMAND echo -en ${FG_GREEN}${ARGN}\n${COLOR_RESET})
endfunction()
# message yellow
function(msg_yellow)
    execute_process(COMMAND echo -en ${FG_YELLOW}${ARGN}\n${COLOR_RESET})
endfunction()
# message blue
function(msg_blue)
    execute_process(COMMAND echo -en ${FG_BLUE}${ARGN}\n${COLOR_RESET})
endfunction()

# find package and specify 
macro(local_find_package)
    # 第一个参数作为包名
    set(pkgname ${ARGV0})

    find_package(${ARGN} QUIET)

    if(${pkgname}_FOUND)
        msg_green("[FOUNDED_PKG]: ${pkgname}")

        # 尝试收集并显示包的所有重要路径信息
        set(path_vars
            ${pkgname}_INCLUDE_DIRS
            ${pkgname}_INCLUDE_DIR
            ${pkgname}_INCLUDES
            ${pkgname}_LIBRARY
            ${pkgname}_LIBRARIES
            ${pkgname}_DIRS
            ${pkgname}_DIR
            ${pkgname}_ROOT
            ${pkgname}_PATH
            ${pkgname}_CONFIG)

        # 循环检查每个可能的路径变量
        foreach(path_var ${path_vars})
            if(DEFINED ${path_var} AND NOT "${${path_var}}" STREQUAL "")
                msg_blue("  - ${path_var}: ${${path_var}}")
            endif()
        endforeach()

        # 显示版本信息(如果有)
        if(DEFINED ${pkgname}_VERSION AND NOT "${${pkgname}_VERSION}" STREQUAL "")
            msg_blue("  - Version: ${${pkgname}_VERSION}")
        endif()
    else()
        # 包未找到 - 以黄色显示警告信息
        msg_yellow("[UNFOUNDED_PKG]: ${pkgname}")
    endif()
endmacro()

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
    endforeach()

    message(STATUS --${TARGETS})
endfunction()
