#include <gtest/gtest.h>

#include "skutils/threadpool.h"

using namespace sk::utils;

#define CORE_SIZE 2

// 测试构造函数和析构函数
TEST(ThreadPoolTest, ConstructionAndDestruction) {
  { ThreadPool pool(CORE_SIZE); }  // 检查线程池是否能正常析构
}

// 测试任务提交与执行
TEST(ThreadPoolTest, SubmitTask) {
  ThreadPool pool(CORE_SIZE);
  int result = 0;

  auto future = pool.submit([](int a, int b) { return a + b; }, 1, 2);
  ASSERT_NO_THROW(result = future.get());
  EXPECT_EQ(result, 3);
}

// 测试多个任务提交与执行
/*
 * Trouble: coresize too big will blocked;
 */
TEST(ThreadPoolTest, MultipleTasks) {
  ThreadPool pool(CORE_SIZE);
  std::vector<std::future<int>> futures;

  for (int i = 0; i < 1000; ++i) {
    futures.push_back(pool.submit([](int x) { return x * x; }, i));
  }

  for (size_t i = 0; i < futures.size(); ++i) {
    ASSERT_NO_THROW(futures[i].get() == i * i);
  }
}

// 测试异常处理
TEST(ThreadPoolTest, ExceptionHandling) {
  ThreadPool pool(CORE_SIZE);

  bool caught_exception = false;
  try {
    pool.submit([]() { throw std::runtime_error("oops!"); }).get();
  } catch (const std::runtime_error &) {
    caught_exception = true;
  }

  EXPECT_TRUE(caught_exception);
}

// 测试边界情况 - 空任务队列
TEST(ThreadPoolTest, EmptyQueue) {
  ThreadPool pool(CORE_SIZE);

  // 提交一个任务以确保线程不会阻塞
  auto f = pool.submit([]() {});
  f.get();
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
