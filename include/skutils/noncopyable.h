#ifndef SHUAIKAI_UTILS_NON_COPYABLE_H
#define SHUAIKAI_UTILS_NON_COPYABLE_H

class NonCopyable {
  public:
  NonCopyable() = default;
  ~NonCopyable() = default;

  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;

  NonCopyable(NonCopyable&&) = default;
  NonCopyable& operator=(NonCopyable&&) = default;
};

#endif  // SHUAIKAI_UTILS_NON_COPYABLE_H
