#ifndef SK_UTILS_PRINTER_H
#define SK_UTILS_PRINTER_H

/**
 * @file printer.h
 * @brief 提供简单的输出任意类型、测试宏等功能，可以彩色输出；要求 c++20 及以上
 * @usage:
 *      打印：
 *        - 对于常见类型（基本类型、STL container、重载了 << 操作符的类型，以及他们的嵌套，可以直接 print 和 toString
 *        - 对于自定义类型，需要实现一个 toString 函数（返回值为任意可转化为string_view的类型），或者重载 << 操作符
 *        - print(obj) 直接打印
 *        - dump(...) 连续打印
 *        - OUT(obj) 打印 [#obj]: obj的形式
 *      测试：
 *        - ASSERT_STR_EQUAL(expect, actual) 简单的测试
 *               将两者转换为字符串进行比较，字符串要求同“打印”（注意，由于转为字符串比较，所以 1 == “1”）
 *        - ASSERT_ALL_PASSED() 统计用例通过情况
 *        - ASSERT(expr)
 *        - ASSERT_MSG(expr, msg)
 *      其他：
 *        - LINE_BREAKER(msg) 输出一个黄色的分割线，中间是信息
 *        - TODO() 输出需要补全代码的位置信息
 * @copyright Copyright (c) shuaikai 2024
 */
// static_assert(__cplusplus >= 202002L, "To use this file, your cpp version must >= CXX20");

#include <cstring>  // for strlen()
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

#include "config.h"  // for global log_lock

/// MARK: TOOLS

#define ELEM_SEP ","
#define DUMP_SEP "\n"

#define GUARD_LOG sk::utils::SpinLockGuard guard(sk::utils::GlobalInfo::getInstance().globalLogSpinLock)

/// MARK: COLOR

#define ANSI_CLEAR     "\033[0m"
#define ANSI_RED_BG    "\033[0;31m"
#define ANSI_GREEN_BG  "\033[0;32m"
#define ANSI_YELLOW_BG "\033[0;33m"
#define ANSI_BLUE_BG   "\033[0;34m"
#define ANSI_PURPLE_BG "\033[0;35m"

/// MARK: Concepts

namespace sk::utils {

struct ListNode;
struct TreeNode;

#if __cplusplus >= 202002L

template <typename T>
concept LeetcodePointerType = std::is_same_v<ListNode *, T> || std::is_same_v<TreeNode *, T>;

template <typename T>
concept StreamOutable = requires(std::ostream &os, T elem) {
                            { os << elem } -> std::same_as<std::ostream &>;
                        };

template <typename T>
concept Serializable = requires(T obj) {
                           { obj.toString() } -> std::convertible_to<std::string_view>;
                       };

template <typename T>
concept SequentialContainer = requires(T c) {
                                  typename T::value_type;
                                  { c.cbegin() } -> std::same_as<typename T::const_iterator>;
                                  { c.cend() } -> std::same_as<typename T::const_iterator>;
                              };

template <typename T>
concept MappedContainer = requires(T m) {
                              typename T::key_type;
                              typename T::mapped_type;
                              { m.cbegin() } -> std::same_as<typename T::const_iterator>;
                              { m.cend() } -> std::same_as<typename T::const_iterator>;
                          };

template <typename T>
concept StackLike = requires(T m) {
                        typename T::value_type;
                        { m.pop() } -> std::same_as<void>;
                        { m.top() } -> std::convertible_to<typename T::const_reference>;
                        { m.empty() } -> std::same_as<bool>;
                    };

template <typename T>
concept QueueLike = requires(T m) {
                        typename T::value_type;
                        { m.pop() } -> std::same_as<void>;
                        { m.front() } -> std::convertible_to<typename T::const_reference>;
                        { m.empty() } -> std::same_as<bool>;
                    };

template <typename T>
concept PairLike = requires(T p) {
                       { std::get<0>(p) } -> std::convertible_to<typename T::first_type>;
                       { std::get<1>(p) } -> std::convertible_to<typename T::second_type>;
                   };

template <typename T>
concept Printable = StreamOutable<T> || Serializable<T> || SequentialContainer<T> || MappedContainer<T> || PairLike<T>
                    || StackLike<T> || QueueLike<T>;

/// MARK: Printer

template <Printable T>
auto toString(const T &obj) -> std::string;

template <typename T>
    requires Printable<typename T::value_type>
auto forBasedContainer2String(const T &c);

template <SequentialContainer T>
    requires Printable<typename T::value_type>
auto SequentialContainer2String(const T &c);

template <PairLike T>
    requires Printable<typename T::first_type> && Printable<typename T::second_type>
auto Pair2String(const T &p);

template <MappedContainer T>
    requires Printable<typename T::key_type> && Printable<typename T::mapped_type>
auto MappedContainer2String(const T &c);

template <StackLike T>
    requires Printable<typename T::value_type>
auto Stack2String(const T &c);

template <QueueLike T>
    requires Printable<typename T::value_type>
auto Queue2String(const T &c);

/// MARK: Printer Impl

template <PairLike T>
    requires Printable<typename T::first_type> && Printable<typename T::second_type>
auto Pair2String(const T &p) {
    std::stringstream ss;
    ss << '{' << toString(std::get<0>(p)) << ELEM_SEP << toString(std::get<1>(p)) << "}";
    return ss.str();
}

template <typename T>
    requires Printable<typename T::value_type>
auto forBasedContainer2String(const T &c) {
    if (c.empty()) {
        return std::string("[]");
    }
    // 会有类型问题？
    // std::accumulate(std::next(c.begin()), c.end(),
    // toString(*(c.begin())),
    // [](string a, auto b){return a + ELEM_SEP + toString(b);});

    std::stringstream ss;
    ss << "[";
    for (const auto &elem : c) {
        ss << toString(elem) << ELEM_SEP;
    }
    std::string ret = ss.str();
    for (int i = 0; i < strlen(ELEM_SEP); ++i) {
        ret.pop_back();
    }
    ret.push_back(']');
    return ret;
}

template <SequentialContainer T>
    requires Printable<typename T::value_type>
auto SequentialContainer2String(const T &c) {
    return forBasedContainer2String(c);
}

template <MappedContainer T>
    requires Printable<typename T::key_type> && Printable<typename T::mapped_type>
auto MappedContainer2String(const T &c) {
    return forBasedContainer2String(c);
}

template <StackLike T>
    requires Printable<typename T::value_type>
auto Stack2String(const T &c) {
    if (c.empty()) {
        return std::string("[]");
    }
    const std::string stack_sep = "<-";
    T                 tmp       = c;
    std::stringstream ss;
    ss << "Stack[" << toString(tmp.top()) << stack_sep;
    tmp.pop();
    while (!tmp.empty()) {
        ss << toString(tmp.top()) << stack_sep;
        tmp.pop();
    }
    std::string ret = ss.str();
    for (int i = 0; i < stack_sep.length(); ++i) {
        ret.pop_back();
    }
    ret.append("]");
    return ret;
}

template <QueueLike T>
    requires Printable<typename T::value_type>
auto Queue2String(const T &c) {
    if (c.empty()) {
        return std::string("[]");
    }
    const std::string queue_sep = "<-";
    T                 tmp       = c;
    std::stringstream ss;
    ss << "Queue[" << toString(tmp.front()) << queue_sep;
    tmp.pop();
    while (!tmp.empty()) {
        ss << toString(tmp.front()) << queue_sep;
        tmp.pop();
    }
    std::string ret = ss.str();
    for (int i = 0; i < queue_sep.length(); ++i) {
        ret.pop_back();
    }
    ret.append("]");
    return ret;
}

template <Printable T>
auto toString(const T &obj) -> std::string {
    if constexpr (LeetcodePointerType<T>) {
        return obj->toString();
    } else if constexpr (Serializable<T>) {
        return obj.toString();
    } else if constexpr (std::is_same_v<T, bool>) {
        return obj ? "True" : "False";
    } else if constexpr (StreamOutable<T>) {
        std::stringstream ss;
        ss << obj;
        return std::move(ss.str());
    } else if constexpr (SequentialContainer<T>) {
        return SequentialContainer2String(obj);
    } else if constexpr (MappedContainer<T>) {
        return MappedContainer2String(obj);
    } else if constexpr (PairLike<T>) {
        return Pair2String(obj);
    } else if constexpr (StackLike<T>) {
        return Stack2String(obj);
    } else if constexpr (QueueLike<T>) {
        return Queue2String(obj);
    } else {
        GUARD_LOG;
        std::cerr << ANSI_RED_BG << "Isn't Printable\n" << ANSI_CLEAR;
        return "@@FALSE_STRING@@";
    }
}

template <Printable T>
void print(const T &obj, const std::string &prefix = "", const std::string &suffix = "", bool lineBreak = true) {
    std::cout << prefix << toString(obj) << suffix;
    if (lineBreak) {
        std::cout << "\n";
    }
}

template <Printable... Args>
void dump(Args... args) {
    ((std::cout << toString(args) << " "), ...);
    std::cout << "\n";
}

template <typename... Args>
std::string format(std::string_view fmt, Args... args) {
    std::string fmtStr(fmt);
    if constexpr (!sizeof...(args)) {
        return fmtStr;
    } else {  //! must has else and constexpr, or you can remove it to see what will happend.
        return ((fmtStr.replace(fmtStr.find("{}"), 2, toString(args))), ...);
    }
}

template <PairLike... PairType>
void dumpWithName(PairType... args) {
    GUARD_LOG;
    ((std::cout << ANSI_PURPLE_BG << "[" << toString(std::get<0>(args)) << "]:" << ANSI_CLEAR
                << toString(std::get<1>(args)) << DUMP_SEP),
     ...);
}

#else  // __cplusplus >= 202002L

/// MARK: CXX 17 Version

using namespace std::string_literals;

template <typename T, typename = void>
struct LeetCodePointerType : std::false_type {};

//* enable_if 作为模板类型来控制模板特化。（另一种常用的方式是作为函数返回值来控制函数特化）
template <typename T>
struct LeetCodePointerType<T, std::enable_if_t<std::is_same_v<ListNode *, T> || std::is_same_v<TreeNode *, T>>>
    : std::true_type {};

template <typename T, typename = void>
struct StreamOutable : std::false_type {};

//* void_t 描述类型特质，具有xx属性
template <typename T>
struct StreamOutable<T, std::void_t<decltype(std::declval<std::ostream &>() << std::declval<T>())>> : std::true_type {};

template <typename T, typename = void>
struct Serializable : std::false_type {};

//* 判断类型属性，也可以使用 enable_if 来实现。如果不关心返回值，则可以使用 void_t
// template <typename T>
// struct Serializable<T, std::void_t<decltype(std::declval<T>().toString())>> : std::true_type {};
template <typename T>
struct Serializable<T, std::enable_if_t<std::is_convertible_v<decltype(std::declval<T>().toString()), std::string>>>
    : std::true_type {};

template <typename T, typename = void>
struct SequentialContainer : std::false_type {};

//* 将 enable_if_t 嵌入到 void_t 中使用。std::void_t 可以接受 任意多 的类型，判断他们是否都存在
template <typename T>
struct SequentialContainer<
    T, std::void_t<typename T::value_type,
                   std::enable_if_t<std::is_same_v<decltype(std::declval<T>().empty()), bool>
                                    && std::is_same_v<decltype(std::declval<T>().cbegin()), typename T::const_iterator>
                                    && std::is_same_v<decltype(std::declval<T>().cend()), typename T::const_iterator>>>>
    : std::true_type {};

template <typename T, typename = void>
struct MappedContainer : std::false_type {};

template <typename T>
struct MappedContainer<
    T, std::void_t<typename T::key_type, typename T::mapped_type,
                   std::enable_if_t<std::is_same_v<decltype(std::declval<T>().cbegin()), typename T::const_iterator>
                                    && std::is_same_v<decltype(std::declval<T>().cend()), typename T::const_iterator>>>>
    : std::true_type {};

template <typename T, typename = void>
struct StackLike : std::false_type {};

//* 使用 std::conjunction 来 AND 判断多个类型的 value 是否为 true;
/**
 *   注意，std::void_t 和 std::conjunction 中都使用了 pop() 等成员函数，这并不赘余！
 *   不能觉得 std::conjunction 中有了就可以了。因为 void_t 中函数不存在时 `SFINAF`, 但是 conjunction
 *   中不存在相应的成员而应要declval则是属于 `编译错误`！
 *   因为conjuction属于继承父类，不是模板参数检查，所以也就不能 SFINAF
 */
template <typename T>
struct StackLike<T, std::void_t<typename T::value_type, decltype(std::declval<T>().pop()),
                                decltype(std::declval<T>().push(std::declval<typename T::value_type>())),
                                decltype(std::declval<T>().top()), decltype(std::declval<T>().empty())>>
    : std::conjunction<std::is_same<decltype(std::declval<T>().pop()), void>,
                       std::is_same<decltype(std::declval<T>().push(std::declval<typename T::value_type>())), void>,
                       std::is_same<decltype(std::declval<T>().top()), typename T::value_type>,
                       std::is_same<decltype(std::declval<T>().empty()), bool>> {};

// 当然，仍然可以使用嵌套 enable_if 的方式来做，看起来似乎更简洁
// template <typename T>
// struct StackLike<
//     T, std::void_t<
//            typename T::value_type,
//            std::enable_if_t<
//                std::is_same_v<decltype(std::declval<T>().pop()), void>
//                && std::is_same_v<decltype(std::declval<T>().empty(), bool)>
//                && std::is_same_v<decltype(std::declval<T>().push(std::declval<typename T::value_type>())), void>
//                && std::is_same_v<std::remove_reference_t<decltype(std::declval<T>().top())>, typename
//                T::value_type>>>>
//     : std::true_type {};

template <typename T, typename = void>
struct QueueLike : std::false_type {};

template <typename T>
struct QueueLike<
    T,
    std::void_t<typename T::value_type,
                std::enable_if_t<std::is_same_v<decltype(std::declval<T>().empty()), bool>
                                 && std::is_convertible_v<decltype(std::declval<T>().front()), typename T::value_type>
                                 && std::is_same_v<decltype(std::declval<T>().pop()), void>>>> : std::true_type {};

template <typename T, typename = void>
struct PairLike : std::false_type {};

template <typename T>
struct PairLike<
    T, std::enable_if_t<std::is_convertible_v<decltype(std::get<0>(std::declval<T>())), typename T::first_type>
                        && std::is_convertible_v<decltype(std::get<1>(std::declval<T>())), typename T::second_type>>>
    : std::true_type {};

template <typename T>
struct Printable : std::disjunction<LeetCodePointerType<T>, StreamOutable<T>, Serializable<T>, SequentialContainer<T>,
                                    MappedContainer<T>, StackLike<T>, QueueLike<T>, PairLike<T>> {};

//* enable_if 作为函数的参数，来限制函数模板选择
template <typename T>
auto toString(const T &obj) -> std::enable_if_t<Printable<T>::value, std::string>;

/// MARK: CXX17 Impl

//* enable_if 直接用作模板参数，注意有个等号
template <typename T, typename = std::enable_if_t<PairLike<T>::value && Printable<typename T::first_type>::value
                                                      && Printable<typename T::second_type>::value,
                                                  void>>
auto Pair2String(const T &p) {
    std::stringstream ss;
    ss << '{' << toString(std::get<0>(p)) << ELEM_SEP << toString(std::get<1>(p)) << "}";
    return ss.str();
}

template <typename T>
std::string forBasedContainer2String(const T &c) {
    if (c.empty()) {
        return "[]"s;
    }

    std::stringstream ss;
    ss << "[";
    for (const auto &elem : c) {
        ss << toString(elem) << ELEM_SEP;
    }
    std::string ret = ss.str();
    for (int i = 0; i < strlen(ELEM_SEP); ++i) {
        ret.pop_back();
    }
    ret.push_back(']');
    return ret;
}

//* enable_if 作为函数的参数，来限制函数模板选择
template <typename T>
auto SequentialContainer2String(const T &c)
    -> std::enable_if_t<SequentialContainer<T>::value && Printable<typename T::value_type>::value, std::string> {
    return forBasedContainer2String(c);
}

template <typename T>
auto MappedContainer2String(const T &c)
    -> std::enable_if_t<MappedContainer<T>::value && Printable<typename T::key_type>::value
                            && Printable<typename T::mapped_type>::value,
                        std::string> {
    return forBasedContainer2String(c);
}

template <typename T>
auto Stack2String(const T &c)
    -> std::enable_if_t<StackLike<T>::value && Printable<typename T::value_type>::value, std::string> {
    if (c.empty()) {
        return "[]"s;
    }
    const std::string stack_sep = "<-";
    T                 tmp       = c;
    std::stringstream ss;
    ss << "Stack[" << toString(tmp.top()) << stack_sep;
    tmp.pop();
    while (!tmp.empty()) {
        ss << toString(tmp.top()) << stack_sep;
        tmp.pop();
    }
    std::string ret = ss.str();
    for (int i = 0; i < stack_sep.length(); ++i) {
        ret.pop_back();
    }
    ret.append("]");
    return ret;
}

template <typename T>
auto Queue2String(const T &c)
    -> std::enable_if_t<QueueLike<T>::value && Printable<typename T::value_type>::value, std::string> {
    if (c.empty()) {
        return "[]"s;
    }
    const std::string queue_sep = "<-";
    T                 tmp       = c;
    std::stringstream ss;
    ss << "Queue[" << toString(tmp.front()) << queue_sep;
    tmp.pop();
    while (!tmp.empty()) {
        ss << toString(tmp.front()) << queue_sep;
        tmp.pop();
    }
    std::string ret = ss.str();
    for (int i = 0; i < queue_sep.length(); ++i) {
        ret.pop_back();
    }
    ret.append("]");
    return ret;
}

template <typename T>
auto toString(const T &obj) -> std::enable_if_t<Printable<T>::value, std::string> {
    if constexpr (LeetCodePointerType<T>::value) {
        return obj->toString();
    } else if constexpr (Serializable<T>::value) {
        return obj.toString();
    } else if constexpr (std::is_same_v<T, bool>) {
        return obj ? "True"s : "False"s;
    } else if constexpr (StreamOutable<T>::value) {
        std::stringstream ss;
        ss << obj;
        return std::move(ss.str());
    } else if constexpr (SequentialContainer<T>::value) {
        return SequentialContainer2String(obj);
    } else if constexpr (MappedContainer<T>::value) {
        return MappedContainer2String(obj);
    } else if constexpr (PairLike<T>::value) {
        return Pair2String(obj);
    } else if constexpr (StackLike<T>::value) {
        return Stack2String(obj);
    } else if constexpr (QueueLike<T>::value) {
        return Queue2String(obj);
    } else {
        GUARD_LOG;
        std::cerr << ANSI_RED_BG << "Isn't Printable\n" << ANSI_CLEAR;
        return "@@FALSE_STRING@@";
    }
}

/// MARK: Utils

template <typename T, typename = std::enable_if_t<Printable<T>::value, void>>
auto print(const T &obj, const std::string &prefix = "", const std::string &suffix = "", bool lineBreak = true) {
    std::cout << prefix << toString(obj) << suffix;
    if (lineBreak) {
        std::cout << "\n";
    }
}

template <typename... Args>
void dump(Args... args) {
    ((std::cout << toString(args) << " "), ...);
    std::cout << "\n";
}

template <typename... Args>
std::string format(std::string_view fmt, Args... args) {
    std::string fmtStr(fmt);
    if constexpr (!sizeof...(args)) {
        return fmtStr;
    } else {  //! must has else and constexpr, or you can remove it to see what will happend.
        return ((fmtStr.replace(fmtStr.find("{}"), 2, toString(args))), ...);
    }
}

//* 用 conjunction 来判断可变参数列表
template <typename... PairType, typename = std::enable_if_t<std::conjunction_v<PairLike<PairType>...>, void>>
void dumpWithName(PairType... args) {
    GUARD_LOG;
    ((std::cout << ANSI_PURPLE_BG << "[" << toString(std::get<0>(args)) << "]:" << ANSI_CLEAR
                << toString(std::get<1>(args)) << DUMP_SEP),
     ...);
}

#endif  // __cplusplus >= 202002L

}  // namespace sk::utils

#endif  // SK_UTILS_PRINTER_H
