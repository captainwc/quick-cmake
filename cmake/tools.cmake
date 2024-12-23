# ANSI COLOR
set(FG_RED    \\033[0;31m)
set(FG_GREEN  \\033[0;32m)
set(FG_YELLOW \\033[0;33m)
set(FG_BLUE   \\033[0;36m)
set(BG_RED    \\033[41m)
set(BG_GREEN  \\033[42m)
set(BG_YELLOW \\033[43m)
set(BG_BLUE   \\033[44m)
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

# find package and specify message
macro(local_find_package)
    set(pkgname ${ARGV0})
    find_package(${ARGN} QUIET)
    if(${pkgname}_FOUND)
        msg_green("[FOUNDED_PKG]: ${pkgname}" ${${pkgname}_INCLUDE_DIRS})
    else()
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
