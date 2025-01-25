#include <gtest/gtest.h>

#include <filesystem>
#include <iostream>

#include "skutils/file.h"

using namespace sk::utils;

inline const fs::path test_file{fs::current_path() / "test_gtest_file"};

TEST(FileInfoTest, CommonInfo) {
    file::FileInfo info(test_file);

    EXPECT_TRUE(info.Exists());

    EXPECT_FALSE(info.Empty());

    EXPECT_LE(525576, info.FileSize());
}

TEST(FileReaderTest, Read) {
    file::FileReader reader(test_file);

    EXPECT_TRUE(reader.Exists());

    auto line = reader.ReadLine();
    EXPECT_NE(0, line.size());

    auto content = reader.ReadAll();
    EXPECT_EQ(content.size(), reader.FileSize());
}