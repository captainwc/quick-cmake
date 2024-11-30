#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <source_location>

//! *_mt 多线程线程安全
//! *_st 单线程，不加锁，性能好

int main() {
    auto DemoLogger  = spdlog::stdout_color_mt("DemoLogger");  // 可以定义一个带名字的标准输出logger
    auto file_logger = spdlog::basic_logger_st("file_logger", "log/file_logger.log");

    // default logger
    spdlog::set_level(spdlog::level::debug);  // 设置全局日志级别为 debug
    spdlog::trace("This is a trace message.");
    spdlog::info("Welcome to spdlog!");
    spdlog::debug("This is a debug message.");
    spdlog::warn("User: {}. Time: {}. Message: {}", "Kimi", "2024-11-07", "This is a formatted log message.");
    spdlog::error("This is an error message.");

    // named logger
    DemoLogger->critical("DemoLogger logger example");

    // Macro logger has extra information, ref https://www.cnblogs.com/lidabo/p/16902710.html
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][%n][%s:%#:%!][%l]: %v");
    SPDLOG_LOGGER_WARN(DemoLogger, "This is a warning message.");
    SPDLOG_ERROR("An error occurred");

    // file logger
    file_logger->critical("File logger example");

    return 0;
}