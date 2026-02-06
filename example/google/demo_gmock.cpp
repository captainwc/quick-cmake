#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

// 定义一个接口类
class Database {
  public:
  virtual ~Database() {}

  virtual bool connect(const std::string& connection_string) = 0;
  virtual void disconnect() = 0;
  virtual int getConnectionCount() const = 0;

  virtual std::vector<std::string> query(const std::string& query_string) = 0;
  virtual bool insert(const std::string& table, const std::vector<std::string>& values) = 0;
};

// 创建 Mock 类
class MockDatabase : public Database {
  public:
  MOCK_METHOD(bool, connect, (const std::string& connection_string), (override));
  MOCK_METHOD(void, disconnect, (), (override));
  MOCK_METHOD(int, getConnectionCount, (), (const, override));
  MOCK_METHOD(std::vector<std::string>, query, (const std::string& query_string), (override));
  MOCK_METHOD(bool, insert, (const std::string& table, const std::vector<std::string>& values), (override));
};

// 被测试的类
class UserRepository {
  public:
  UserRepository(Database* db) : db_(db) {}

  bool addUser(const std::string& username, const std::string& email) {
    //* ERROR: 这里EXPECT_CALL里面设置预期会调用Connect，实际上没调用

    // if (!db_->connect("user_database")) {
    //     return false;
    // }

    std::vector<std::string> values = {username, email};
    bool result = db_->insert("users", values);

    db_->disconnect();
    return result;
  }

  std::vector<std::string> getAllUsers() {
    if (!db_->connect("user_database")) {
      return {};
    }

    auto users = db_->query("SELECT * FROM users");
    db_->disconnect();
    return users;
  }

  int getActiveConnections() { return db_->getConnectionCount(); }

  private:
  Database* db_;
};

// 测试类
TEST(UserRepositoryTest, AddUserSuccess) {
  // 创建 mock 对象
  MockDatabase mockDb;

  // 设置期望行为 (使用 EXPECT_CALL)
  EXPECT_CALL(mockDb, connect("user_database")).WillOnce(testing::Return(true));

  std::vector<std::string> expectedValues = {"testuser", "test@example.com"};
  EXPECT_CALL(mockDb, insert("users", expectedValues)).WillOnce(testing::Return(true));

  EXPECT_CALL(mockDb, disconnect()).Times(1);

  // 执行测试
  UserRepository repo(&mockDb);
  bool result = repo.addUser("testuser", "test@example.com");

  // 验证结果
  EXPECT_TRUE(result);
}

TEST(UserRepositoryTest, AddUserFailedConnection) {
  // 创建 mock 对象并设置严格模式
  testing::StrictMock<MockDatabase> mockDb;

  // 设置期望行为 - 连接失败
  EXPECT_CALL(mockDb, connect("user_database")).WillOnce(testing::Return(false));

  // insert 和 disconnect 方法不应该被调用

  // 执行测试
  UserRepository repo(&mockDb);
  bool result = repo.addUser("testuser", "test@example.com");

  // 验证结果
  EXPECT_FALSE(result);
}

TEST(UserRepositoryTest, GetAllUsers) {
  // 创建 mock 对象
  MockDatabase mockDb;

  // 设置行为顺序
  {
    testing::InSequence seq;

    EXPECT_CALL(mockDb, connect("user_database")).WillOnce(testing::Return(true));

    std::vector<std::string> mockResults = {"user1", "user2", "user3"};
    EXPECT_CALL(mockDb, query("SELECT * FROM users")).WillOnce(testing::Return(mockResults));

    EXPECT_CALL(mockDb, disconnect());
  }

  // 执行测试
  UserRepository repo(&mockDb);
  auto users = repo.getAllUsers();

  // 验证结果
  ASSERT_EQ(users.size(), 3);
  EXPECT_EQ(users[0], "user1");
  EXPECT_EQ(users[1], "user2");
  EXPECT_EQ(users[2], "user3");
}

TEST(UserRepositoryTest, GetActiveConnections) {
  // 创建 mock 对象
  MockDatabase mockDb;

  // 使用 WillRepeatedly 设置重复返回值
  EXPECT_CALL(mockDb, getConnectionCount()).WillRepeatedly(testing::Return(5));

  // 执行测试
  UserRepository repo(&mockDb);
  int connections = repo.getActiveConnections();

  // 验证结果
  EXPECT_EQ(connections, 5);
}

TEST(UserRepositoryTest, ArgumentMatchers) {
  // 创建 mock 对象
  MockDatabase mockDb;

  // 使用不同的参数匹配器
  EXPECT_CALL(mockDb, connect(testing::StartsWith("user_"))).WillOnce(testing::Return(true));

  EXPECT_CALL(mockDb, insert(testing::_, testing::SizeIs(2))).WillOnce(testing::Return(true));

  EXPECT_CALL(mockDb, disconnect());

  // 执行测试
  UserRepository repo(&mockDb);
  bool result = repo.addUser("testuser", "test@example.com");

  // 验证结果
  EXPECT_TRUE(result);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}