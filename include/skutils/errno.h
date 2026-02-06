#ifndef SHUAIKAI_UTILS_ERRNO_H
#define SHUAIKAI_UTILS_ERRNO_H

#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <cerrno>
#endif

#include <system_error>

#include "skutils/printer.h"
#include "skutils/string_utils.h"

namespace sk::utils {

static void throwLastError(const char* context, const char* file, const char* function, int line) {
#ifdef _WIN32
  auto code = WSAGetLastError();
#else
  auto code = errno;
#endif

  auto msg = sk::utils::format("[{}:{}:{}][{}]", sk::utils::str::basenameWithoutExt(file), function, line, context);

  throw std::system_error(code, std::system_category(), msg);
}

#define ThrowLastError(context) throwLastError(context, __FILE__, __FUNCTION__, __LINE__)

}  // namespace sk::utils

#endif  // SHUAIKAI_UTILS_ERRNO_H
