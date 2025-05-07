#ifndef SK_UTILS_STACK_PRINTER_H
#define SK_UTILS_STACK_PRINTER_H

#pragma once

#if !defined(__unix__) && !defined(__linux__)
#error "Only aviable on unix"
#endif

#define SUPPORT_COLORFUL_OUTPUT

#include <cxxabi.h>
#include <execinfo.h>

#include <cstdio>
#include <sstream>
#include <string>
#include <thread>

#define PRINT_STACK_HERE sk::utils::dbg::StackTracer::PrintStack(__FILE__, __func__, __LINE__)

namespace sk::utils::dbg {

class StackTracer {
public:
    static std::string GetStack(bool withColor = false);
    static std::string Demagle(const char* symbol_name);

    static void PrintStack(const char* file, const char* func, int line);

private:
    constexpr static uint MAX_STACK_SIZE = 100;

    static std::string ThreadID();
    static std::string ParseSymbolEntry(const char* entry, bool isColorful);

    constexpr static const char* ANSI_RESET  = "\033[0m";
    constexpr static const char* ANSI_GREEN  = "\033[1;32m";
    constexpr static const char* ANSI_BLUE   = "\033[1;34m";
    constexpr static const char* ANSI_YELLOW = "\033[1;33m";
    constexpr static const char* ANSI_PURPLE = "\033[1;35m";
    constexpr static const char* ANSI_CYAN   = "\033[1;36m";
    constexpr static const char* ANSI_GREY   = "\033[1;37m";

    constexpr static const char* STACK_ENTRY_OF_THIS_FILE = "STACK_ENTRY_OF_THIS_FILE";
    constexpr static const char* EMPTY_STACK_ENTRY        = "EMPTY_STACK_ENTRY";
    constexpr static const char* INVALID_SYMBOL_ENTRY     = "INVALID_SYMBOL_ENTRY";
};

template <typename T>
inline std::string TypeName() {
    return sk::utils::dbg::StackTracer::Demagle(typeid(T).name());
}

template <typename T>
inline std::string TypeName(T /*e*/) {
    return sk::utils::dbg::StackTracer::Demagle(typeid(T).name());
}

inline std::string StackTracer::GetStack(bool withColor) {
    void* buf[MAX_STACK_SIZE];
    int   ret = backtrace(buf, MAX_STACK_SIZE);
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
        auto ret = ParseSymbolEntry(symbols[i], withColor);
        if (ret == STACK_ENTRY_OF_THIS_FILE) {
            continue;
        }
        ss << " " << stack_entry_id++ << "# " << ret << "\n";
    }
    free(symbols);
    return ss.str();
}

inline void StackTracer::PrintStack(const char* file, const char* func, int line) {
#ifdef SUPPORT_COLORFUL_OUTPUT
    printf("%s====== STACK AT [%s%s%s][%s%s:%s:%d%s] ======%s\n%s\n", ANSI_YELLOW, ANSI_PURPLE, ThreadID().c_str(),
           ANSI_YELLOW, ANSI_GREEN, file, func, line, ANSI_YELLOW, ANSI_RESET, GetStack(true).c_str());
#elif
    printf("====== STACK AT [%s][%s:%s:%d] ======\n%s\n", ThreadID.c_str(), file, func, line, GetStack(false).c_str());
#endif
}

inline std::string StackTracer::ParseSymbolEntry(const char* entry, bool isColorful) {
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
    auto executable_name = symbol.substr(0, pos1);
    auto symbol_name     = symbol.substr(pos1 + 1, pos2 - pos1 - 1);
    auto demagled_name   = Demagle(symbol_name.c_str());
    // Filterd stack in this file
    if (demagled_name.find("sk::utils::dbg::") != std::string::npos) {
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

inline std::string StackTracer::Demagle(const char* symbol_name) {
    int   status         = 0;
    char* demangled_name = abi::__cxa_demangle(symbol_name, nullptr, nullptr, &status);
    if (status != 0) {
        return symbol_name;
    }
    std::string ret = demangled_name;
    free(demangled_name);
    return ret;
}

inline std::string StackTracer::ThreadID() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

}  // namespace sk::utils::dbg

#endif  // SK_UTILS_STACK_PRINTER_H
