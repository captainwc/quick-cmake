#include <gtest/gtest.h>

#include <iostream>

#include "skutils/file.h"

using namespace sk::utils;

inline const fs::path test_file(file::FileInfo::HomeDir() + "/.bashrc");

TEST(FileInfoTest, CommonInfo) {
    file::FileInfo info(test_file);

    EXPECT_TRUE(info.Exists());

    EXPECT_FALSE(info.Empty());

    EXPECT_EQ(3755, info.FileSize());
}

TEST(FileReaderTest, Read) {
    file::FileReader reader(test_file);

    EXPECT_TRUE(reader.Exists());

    auto line = reader.ReadLine();
    EXPECT_NE(0, line.size());

    auto content = reader.ReadAll();
    EXPECT_EQ(content.size(), reader.FileSize());
}