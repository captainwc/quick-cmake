msg_red("Coverage ON")

# 查找 lcov 可执行文件路径
find_program(LCOV_PATH lcov REQUIRED)

# 获取 lcov 版本
execute_process(
    COMMAND ${LCOV_PATH} --version
    OUTPUT_VARIABLE LCOV_VERSION_OUTPUT
    ERROR_QUIET
    RESULT_VARIABLE LCOV_VERSION_RESULT)

# 检查 lcov 版本
if(LCOV_VERSION_RESULT EQUAL 0)
    string(REGEX MATCH "([0-9]+)\\.([0-9]+)" _match ${LCOV_VERSION_OUTPUT})
    if(_match)
        set(LCOV_MAJOR_VERSION ${CMAKE_MATCH_1})
        set(LCOV_MINOR_VERSION ${CMAKE_MATCH_2})
        # 2.0 版本需要忽略mismatch, 1.5版本又不支持这个参数
        if(${LCOV_MAJOR_VERSION} GREATER 1)
            set(LCOV_EXTRA_ARGS --ignore-errors mismatch --ignore-errors unused --ignore-errors negative)
        endif()
    endif()
else()
    message(FATAL_ERROR "Failed to get lcov version")
endif()

# 最终使用 lcov 时加上参数
msg_red("LCOV extra args: ${LCOV_EXTRA_ARGS}")

find_program(GENHTML_PATH genhtml REQUIRED)
find_program(GCOV_PATH gcov)

# 添加编译器标志以启用覆盖率
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")

# 创建一个清理覆盖率数据的目标
add_custom_target(
    coverage-clean-init
    COMMAND ${LCOV_PATH} ${LCOV_EXTRA_ARGS} --rc lcov_branch_coverage=1 --zerocounters --directory .
    COMMAND ${LCOV_PATH} ${LCOV_EXTRA_ARGS} --rc lcov_branch_coverage=1 --capture --initial --directory . --output-file
            coverage.base
    COMMENT "Cleaning up old coverage data and capturing baseline")

# 定义复制脚本的命令
set(RUN_TESTS_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/run_coverage_target.sh")
set(BUILD_RUN_TESTS_SCRIPT "${CMAKE_BINARY_DIR}/run_coverage_target.sh")
add_custom_command(
    OUTPUT ${BUILD_RUN_TESTS_SCRIPT}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RUN_TESTS_SCRIPT} ${BUILD_RUN_TESTS_SCRIPT}
    COMMENT "Copying run_coverage_target.sh to build directory"
    VERBATIM)

# 创建一个运行测试并捕获覆盖率数据的目标
add_custom_target(
    coverage-tests-run
    DEPENDS coverage-clean-init
    # DEPENDS ${BUILD_RUN_TESTS_SCRIPT}
    # COMMAND bash ${BUILD_RUN_TESTS_SCRIPT}
    COMMAND cd ${CMAKE_BINARY_DIR} && ctest --output-on-failure -j10
    COMMAND ${LCOV_PATH} ${LCOV_EXTRA_ARGS} --rc lcov_branch_coverage=1 --directory . --capture --output-file
            coverage.test
    COMMENT "Running tests and capturing test coverage data")

# 创建一个合并覆盖率数据并生成报告的目标
add_custom_target(
    coverage-report
    DEPENDS coverage-tests-run
    COMMAND ${LCOV_PATH} ${LCOV_EXTRA_ARGS} --rc lcov_branch_coverage=1 --add-tracefile coverage.base --add-tracefile
            coverage.test --output-file coverage.total
    # COMMAND ${LCOV_PATH} ${LCOV_EXTRA_ARGS} --rc lcov_branch_coverage=1 --remove coverage.total '/usr/*'
    # '*/third_party/*' '*/build/*' --output-file coverage.filtered.info
    COMMAND ${LCOV_PATH} ${LCOV_EXTRA_ARGS} --rc lcov_branch_coverage=1 --extract coverage.total '*/include/skutils/*'
            --output-file coverage.filtered.info
    COMMAND ${GENHTML_PATH} --branch-coverage coverage.filtered.info --output-directory coverage_report
    COMMENT "Generating code coverage report")

# 创建一个总的目标，可以一键完成所有步骤
add_custom_target(coverage DEPENDS coverage-report COMMENT "Generating full code coverage report")
