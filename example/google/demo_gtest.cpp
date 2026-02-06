#define DBG_MACRO_NO_WARNING
#include <dbg.h>
#include <gtest/gtest-typed-test.h>
#include <gtest/gtest.h>
#include <skutils/printer.h>

#include <iostream>
#include <map>
#include <unordered_map>

// ============== 简单直接的测试用例 ==============
TEST(DemoTest, FirstDemo) {
  // 推荐使用大括号来分割测试用例，但是也注意不要然一个用例膨胀

  {
    int x = 10;
    EXPECT_EQ(10, x);
  }

  {
    int x = 11;
    ASSERT_EQ(11, x);
  }
}

// ============== 测试夹具， 允许执行一些前后操作 ==============
class MapTest : public ::testing::Test {
  protected:
  std::unordered_map<int, int> ump;  // for each test fixture

  static void SetUpTestSuite() {}

  static void TearDownTestSuite() {}

  void SetUp() override { ump.emplace(2, 0); }

  void TearDown() override {}
};

TEST_F(MapTest, insert) {
  ump.emplace(1, 3);
  EXPECT_EQ(2, ump.size());
}

TEST_F(MapTest, AfterInsert) {
  EXPECT_EQ(1, ump.size());  // 此处的 ump 是全新的，不受之前 testcase 中插入的影响
}

// =========== 类型测试， 自动测试不同类型 ===============

// (1) 声明测试夹具。使用TYPED_TEST 必须声明夹具，而且是模板类，即使是空的
template <typename T>
class PrinterTest : public ::testing::Test {};

// (2) 指定各种待测试类型
using TypesToPrint = ::testing::Types<int, float, std::string, const char*, std::vector<int>,
                                      std::map<int, std::string>, std::map<std::vector<int>, const char*>>;

// (3) 注册类型和夹具
TYPED_TEST_SUITE(PrinterTest, TypesToPrint);

// (4) 开始使用，定义通用测试逻辑
TYPED_TEST(PrinterTest, JustToString) {
  // 使用 TypeParam 来指代待测类型
  TypeParam elem;
  auto elem2string = sk::utils::toString(elem);
  // 这里因为没啥好测试的，转换失败就是编译不通过了，所以直接打印出来好了
  dbg(elem);
  dbg(elem2string);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
