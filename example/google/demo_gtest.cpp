#include <gtest/gtest.h>

#include <unordered_map>

class MapTest : public ::testing::Test {
protected:
    std::unordered_map<int, int> ump;  // for each test fixture

    static void SetUpTestSuite() {}

    static void TearDownTestSuite() {}

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(MapTest, insert) {
    ump.emplace(1, 3);
    EXPECT_EQ(1, ump.size());
}

TEST_F(MapTest, after_insert) {
    EXPECT_EQ(1, ump.size());  // 此处的 ump 是全新的，不受之前 testcase 中插入的影响
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}