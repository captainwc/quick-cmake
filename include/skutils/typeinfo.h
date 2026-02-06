#pragma once

#ifndef SK_UTILS_TYPE_INFO_H
#define SK_UTILS_TYPE_INFO_H

#include <cstdint>
#include <string_view>

namespace sk::utils::type {

using TypeID = std::uint64_t;

constexpr TypeID Hash(std::string_view str) {
  TypeID hash = 14695981039346656037ull;
  for (char c : str) {
    hash ^= static_cast<TypeID>(c);
    hash *= 1099511628211ull;
  }
  return hash;
}

template <typename T>
constexpr std::string_view GetTypeNameRaw() {
#if defined(__clang__) || defined(__GNUC__)
  return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
  return __FUNCSIG__;
#else
#error "Unsupported compiler"
#endif
}

template <typename T>
constexpr TypeID GetTypeID() {
  return Hash(GetTypeNameRaw<T>());
}
}  // namespace sk::utils::type

#endif  // SK_UTILS_TYPE_INFO_H
