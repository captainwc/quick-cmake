#ifndef SK_UTILS_STACK_PRINTER_H
#define SK_UTILS_STACK_PRINTER_H

#pragma once

#if !defined(__unix__) || !defined(__linux__)
#error \
    "This can only be used in unix system. (For windows or mingw, Firstlt you should have boost, and then modify relavant lines by yourself."
#endif

//* Choices Begin
#define SK_DEBUG_MODE               // close this to make PRINT_STACK_HERE do nothing
#define STACKTRACE_OUTPUT_COLORFUL  // toggle PRINT_STACK_HERE colorful output
// #define STACKTRACE_OUTPUT_FULLPATH // print full path rather than just basename
// #define STACKTRACE_NO_WARNING // to close #warning
// #define I_HAVE_BOOST  // to use boost::stacktrace
//* Choices End

#ifndef SK_DEBUG_MODE
#define PRINT_STACK_HERE
#else

#if __cxxplus >= 202300L
#ifndef STACKTRACE_NO_WARNING
#warning \
    "Use [cpp23 <stacktrace>] to get stack. Remember to link with `g++ -lstdc++exp` or `clang++ -lc++experimental`. Define `STACKTRACE_NO_WARNING` to disable this warning."
#endif
#include <stacktrace>
#elif defined(I_HAVE_BOOST)
#ifndef STACKTRACE_NO_WARNING
#warning \
    "Use [boost stacktrace] to get stack. Remember to link with `-g`. Define `STACKTRACE_NO_WARNING` to disable this warning."
#endif
// Use addr2line wrapper provided by boost for default.
// You can ref the link for more information about this macro:
// https://www.boost.org/doc/libs/master/doc/html/stacktrace/configuration_and_build.html#stacktrace.configuration_and_build.header_only_options
#define BOOST_STACKTRACE_USE_ADDR2LINE
#include <boost/stacktrace.hpp>
#else
#ifndef STACKTRACE_NO_WARNING
#warning \
    "Use [backtrace] to get stack. Remember to link with `-rdynamic`. Define `STACKTRACE_NO_WARNING` to disable this warning."
#endif
#include <execinfo.h>
#endif

#include <cxxabi.h>

#include <cstdio>
#include <sstream>
#include <string>
#include <thread>  // for std::this_thread::get_id

//* Core API Begin
#ifdef STACKTRACE_OUTPUT_COLORFUL
#define PRINT_STACK_HERE sk::utils::dbg::PrintCurrentStack(__FILE__, __func__, __LINE__, true)
#else
#define PRINT_STACK_HERE sk::utils::dbg::PrintCurrentStack(__FILE__, __func__, __LINE__, false)
#endif

namespace sk::utils::dbg {

inline std::string GetCurrentStack(bool withColor);

inline void PrintCurrentStack(const char* file, const char* func, int line, bool withColor);
}  // namespace sk::utils::dbg

//* Core API End

/// MARK: Implementations

namespace sk::utils::dbg::details {
constexpr const char* ANSI_RESET  = "\033[0m";
constexpr const char* ANSI_GREEN  = "\033[1;32m";
constexpr const char* ANSI_BLUE   = "\033[1;34m";
constexpr const char* ANSI_YELLOW = "\033[1;33m";
constexpr const char* ANSI_PURPLE = "\033[1;35m";
constexpr const char* ANSI_GREY   = "\033[1;37m";

inline std::string threadID() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

inline std::string fileBaseName(std::string filename) {
    auto pos = filename.rfind('/');
    if (pos == std::string::npos) {
        return filename;
    }
    return filename.substr(pos + 1);
}

#if __cplusplus >= 202300L
inline std::string getStackWithCpp23(bool withColor) {
    std::stringstream ss;
    int               stack_entry_id = 0;
    for (auto& entry : std::source_location::current()) {
        ss << " " << stack_entry_id++ << "# " << entry << "\n";
    }
    return ss.str();
}
#elif defined(I_HAVE_BOOST)
inline std::string getStackWithBoostStacktrace(bool withColor) {
    std::stringstream ss;
    int               stack_entry_id = 0;
    for (auto& entry : boost::stacktrace::stacktrace()) {
        auto func = entry.name();
        if (func.find("sk::utils::dbg") != std::string::npos || func.find("boost::stacktrace") != std::string::npos) {
            continue;
        }
#ifdef STACKTRACE_OUTPUT_FULLPATH
        auto source_file = entry.source_file();
#else
        auto source_file = fileBaseName(entry.source_file());
#endif
        if (source_file.empty()) {
            std::stringstream tmp;
            tmp << entry;
            auto entrystr = tmp.str();
            auto pos      = entrystr.find(" in ");
#ifdef STACKTRACE_OUTPUT_FULLPATH
            source_file   = entrystr.substr(pos + 4);
#else
            source_file = fileBaseName(entrystr.substr(pos + 4));
#endif
        }

        ss << " " << stack_entry_id++ << "# ";
        if (withColor) {
            ss << ANSI_GREY << "[" << source_file << ":" << entry.source_line() << "] " << ANSI_BLUE << entry.name()
               << ANSI_RESET << " [" << entry.address() << "]"
               << "\n";
        } else {
            ss << "[" << source_file << ":" << entry.source_line() << "] " << entry.name() << " [" << entry.address()
               << "]"
               << "\n";
        }
    }

    return ss.str();
}
#else

constexpr uint        MAX_STACK_SIZE           = 100;
constexpr const char* STACK_ENTRY_OF_THIS_FILE = "STACK_ENTRY_OF_THIS_FILE";
constexpr const char* EMPTY_STACK_ENTRY        = "EMPTY_STACK_ENTRY";
constexpr const char* INVALID_SYMBOL_ENTRY     = "INVALID_SYMBOL_ENTRY";

inline std::string demangle(const char* symbol_name) {
    int   status         = 0;
    char* demangled_name = abi::__cxa_demangle(symbol_name, nullptr, nullptr, &status);
    if (status != 0) {
        return symbol_name;
    }
    std::string ret = demangled_name;
    free(demangled_name);
    return ret;
}

inline std::string parseSymbolEntryOfBacktrace(const char* entry, bool isColorful) {
    if (entry == nullptr) {
        return EMPTY_STACK_ENTRY;
    }
    std::string symbol = entry;
    auto        pos1   = symbol.find('(');
    auto        pos2   = symbol.find('+');
    auto        pos3   = symbol.find(')');
    if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos) {
        return INVALID_SYMBOL_ENTRY;
    }
#ifdef STACKTRACE_OUTPUT_FULLPATH
    auto executable_name = symbol.substr(0, pos1);
#else
    auto executable_name = fileBaseName(symbol.substr(0, pos1));
#endif
    auto symbol_name     = symbol.substr(pos1 + 1, pos2 - pos1 - 1);
    auto demagled_name   = details::demangle(symbol_name.c_str());
    // Filterd stack in this file
    if (demagled_name.find("dbg::") != std::string::npos) {
        return STACK_ENTRY_OF_THIS_FILE;
    }

    auto offset = symbol.substr(pos2 + 1, pos3 - pos2 - 1);

    auto        pos4 = symbol.rfind('[');
    auto        pos5 = symbol.rfind(']');
    std::string address;
    if (pos4 != std::string::npos && pos5 != std::string::npos) {
        address = symbol.substr(pos4 + 1, pos5 - pos4 - 1);
    }
    std::stringstream ss;
    if (isColorful) {
        ss << ANSI_GREY << "[" << executable_name << "] " << ANSI_BLUE << demagled_name << ANSI_RESET << " : " << offset
           << " [" << address << "]";
    } else {
        ss << "[" << executable_name << "] " << demagled_name << " : " << offset << " [" << address << "]";
    }
    return ss.str();
}

inline std::string getStackWithBacktrace(bool withColor) {
    void* buf[details::MAX_STACK_SIZE];
    int   ret = backtrace(buf, details::MAX_STACK_SIZE);
    if (ret == 0) {
        return "";
    }
    char** symbols = backtrace_symbols(buf, ret);
    if (symbols == nullptr) {
        return "";
    }
    std::stringstream ss;
    int               stack_entry_id = 0;
    for (int i = 0; i < ret; ++i) {
        auto ret = details::parseSymbolEntryOfBacktrace(symbols[i], withColor);
        if (ret == details::STACK_ENTRY_OF_THIS_FILE) {
            continue;
        }
        ss << " " << stack_entry_id++ << "# " << ret << "\n";
    }
    free(symbols);
    return ss.str();
}
#endif

}  // namespace sk::utils::dbg::details

namespace sk::utils::dbg {
inline std::string GetCurrentStack(bool withColor) {
#if __cplusplus >= 202300L
    return details::getStackWithCpp23(withColor);
#elif defined(I_HAVE_BOOST)
    return details::getStackWithBoostStacktrace(withColor);
#else
    return details::getStackWithBacktrace(withColor);
#endif
}

inline void PrintCurrentStack(const char* file, const char* func, int line, bool withColor) {
    if (withColor) {
        printf("%s====== STACK AT [%s%s%s][%s%s:%s:%d%s] ======%s\n%s\n", details::ANSI_YELLOW, details::ANSI_PURPLE,
               details::threadID().c_str(), details::ANSI_YELLOW, details::ANSI_GREEN, file, func, line,
               details::ANSI_YELLOW, details::ANSI_RESET, GetCurrentStack(true).c_str());
    } else {
        printf("====== STACK AT [%s][%s:%s:%d] ======\n%s\n", details::threadID().c_str(), file, func, line,
               GetCurrentStack(false).c_str());
    }
}
}  // namespace sk::utils::dbg

#endif  // SK_DEBUG_MODE

#endif  // SK_UTILS_STACK_PRINTER_H
