msg_red("Coverage ON")

find_program(LCOV_PATH lcov REQUIRED)
find_program(GENHTML_PATH genhtml REQUIRED)
find_program(GCOV_PATH gcov)

# 添加编译器标志以启用覆盖率
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")

# 创建一个清理覆盖率数据的目标
add_custom_target(
    coverage-clean-init
    COMMAND ${LCOV_PATH} --zerocounters --directory .
    COMMAND ${LCOV_PATH} --capture --initial --directory . --output-file coverage.base
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
    DEPENDS ${BUILD_RUN_TESTS_SCRIPT}
    COMMAND bash ${BUILD_RUN_TESTS_SCRIPT}
    COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.test
    COMMENT "Running tests and capturing test coverage data")

# 创建一个合并覆盖率数据并生成报告的目标
add_custom_target(
    coverage-report
    DEPENDS coverage-tests-run
    COMMAND ${LCOV_PATH} --add-tracefile coverage.base --add-tracefile coverage.test --output-file coverage.total
    COMMAND ${LCOV_PATH} --remove coverage.total '/usr/*' '*/third_party/*' '*/build/*' --output-file
            coverage.filtered.info
    # COMMAND ${LCOV_PATH} --extract coverage.total '*/include/skutils/*' --output-file coverage.filtered.info
    COMMAND ${GENHTML_PATH} coverage.filtered.info --output-directory coverage_report
    COMMENT "Generating code coverage report")

# 创建一个总的目标，可以一键完成所有步骤
add_custom_target(coverage DEPENDS coverage-report COMMENT "Generating full code coverage report")
