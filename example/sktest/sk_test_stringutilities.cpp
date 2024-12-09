#include <gtest/gtest.h>

#include "skutils/printer.h"
#include "skutils/string_utilities.h"

TEST(SK_STRING_UTILITIES_TEST, splited_by_char) {  // NOLINT
    std::string s("hello world");
    auto        ret = sk::utils::str::split(s, 'o');
    EXPECT_EQ(sk::utils::toString(ret), "[hell, w,rld]");
}

TEST(SK_STRING_UTILITIES_TEST, splited_by_chars) {  // NOLINT
    std::string s("hello world");
    auto        ret = sk::utils::str::split(s, {'o', 'l'});
    EXPECT_EQ(sk::utils::toString(ret), "[he, w,r,d]");
}

TEST(SK_STRING_UTILITIES_TEST, splited_by_str) {  // NOLINT
    std::string s("hello worlld");
    auto        ret = sk::utils::str::split(s, "ll");
    EXPECT_EQ(sk::utils::toString(ret), "[he,o wor,d]");
}

TEST(SK_STRING_UTILITIES_TEST, splited_by_strs) {  // NOLINT
    std::string s("hello worlld");
    auto        ret = sk::utils::str::split(s, {"ll", "llo", "e"});
    EXPECT_EQ(sk::utils::toString(ret), "[h, wor,d]");
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
