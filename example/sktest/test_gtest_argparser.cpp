#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "skutils/argparser.h"

using namespace sk::utils::arg;

class ArgParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser.add_arg({.name = "--path", .type = ArgType::LIST, .help = "File or Directory"})
            .add_arg({.name = "-o", .type = ArgType::STR, .help = "Out Put Name"})
            .add_arg({.name = "-i", .type = ArgType::INT, .help = "Short Path"})
            .add_arg({.name = "-bool", .type = ArgType::BOOL, .help = "Boolean Value"});
        // .add_arg({.name = "error", .type = ArgType::STR, .help = "Skipped Value"});
    }

    ArgParser parser;
};

TEST_F(ArgParserTest, BasicParsing) {
    int   argc   = 14;
    char* argv[] = {"filename", "default_val", "-o",           "false_outname", "-o=true_outname",
                    "--path",   "path1",       "path space 2", "--path",        "dir/path3",
                    "-i",       "123",         "-bool",        "unparsed",      "out_range"};

    parser.parse(argc, argv);

    EXPECT_EQ(parser.get_file_name(), "filename");

    auto front_args = parser.get_front_args();
    ASSERT_TRUE(front_args.has_value());
    EXPECT_EQ(front_args.value(), std::vector<std::string>{"default_val"});

    auto back_args = parser.get_back_args();
#if COLLECT_UNEXPECTED_ARGS
    ASSERT_TRUE(back_args.has_value());
    EXPECT_EQ(back_args.value(), std::vector<std::string>{"unparsed"});
#else
    EXPECT_FALSE(back_args.has_value());
#endif

    auto out_name = parser.get_value("-o");
    ASSERT_TRUE(out_name.has_value());
    EXPECT_EQ(std::get<std::string>(out_name.value()), "true_outname");

    auto path = parser.get_value("--path");
    ASSERT_TRUE(path.has_value());
    auto path_values = std::get<std::vector<std::string>>(path.value());
    ASSERT_EQ(path_values.size(), 3);
    EXPECT_EQ(path_values[0], "path1");
    EXPECT_EQ(path_values[1], "path space 2");
    EXPECT_EQ(path_values[2], "dir/path3");

    auto int_val = parser.get_value("-i");
    ASSERT_TRUE(int_val.has_value());
    EXPECT_EQ(std::get<int>(int_val.value()), 123);

    auto bool_val = parser.get_value("-bool");
    ASSERT_TRUE(bool_val.has_value());
    EXPECT_TRUE(std::get<bool>(bool_val.value()));
}

TEST_F(ArgParserTest, InvalidArgumentHandling) {
    int   argc   = 3;
    char* argv[] = {"filename", "invalid_arg", "value"};

    parser.parse(argc, argv);

    auto front_args = parser.get_front_args();
    ASSERT_TRUE(front_args.has_value());
    auto front_values = front_args.value();
    ASSERT_EQ(front_values.size(), 2);
    EXPECT_EQ(front_values[0], "invalid_arg");
    EXPECT_EQ(front_values[1], "value");
}

TEST_F(ArgParserTest, MissingRequiredValues) {
    int   argc   = 3;
    char* argv[] = {"filename", "-i", "-o"};

    parser.parse(argc, argv);

    auto int_val = parser.get_value("-i");
    EXPECT_FALSE(int_val.has_value());

    auto str_val = parser.get_value("-o");
    EXPECT_FALSE(str_val.has_value());
}

TEST_F(ArgParserTest, HelpFlag) {
    int   argc   = 2;
    char* argv[] = {"filename", "-h"};

    parser.parse(argc, argv);
    EXPECT_TRUE(parser.need_help());

    argv[1] = "--help";
    parser.parse(argc, argv);
    EXPECT_TRUE(parser.need_help());
}

TEST_F(ArgParserTest, ArgInfoMethods) {
    ArgInfo info{.name = "--test", .type = ArgType::INT, .help = "Test argument", .sname = "-t"};

    EXPECT_EQ(info.type_str(ArgType::INT), "int");
    EXPECT_EQ(info.type_str(ArgType::FLOAT), "float");
    EXPECT_EQ(info.type_str(ArgType::BOOL), "bool");
    EXPECT_EQ(info.type_str(ArgType::STR), "str");
    EXPECT_EQ(info.type_str(ArgType::LIST), "strlist");

    EXPECT_EQ(info.toString(), "-t, --test [int] Test argument");
}

TEST_F(ArgParserTest, GetValueWithDefault) {
    int   argc   = 8;
    char* argv[] = {"filename", "front1", "front2", "--path", "path1", "path2", "back1", "back2"};

    parser.parse(argc, argv);

    auto values = parser.get_value_with_default("--path");
    ASSERT_TRUE(values.has_value());
    auto default_values = values.value();
    ASSERT_EQ(default_values.size(), 6);
    //>mark 这里的顺序是否合适，可以琢磨一下
    EXPECT_EQ(default_values[0], "path1");
    EXPECT_EQ(default_values[1], "path2");
    EXPECT_EQ(default_values[2], "back1");
    EXPECT_EQ(default_values[3], "back2");
    EXPECT_EQ(default_values[4], "front1");
    EXPECT_EQ(default_values[5], "front2");
}

TEST_F(ArgParserTest, InvalidArgName) {
    ArgParser local_parser;
    local_parser.add_arg({.name = "invalid", .type = ArgType::STR, .help = "Invalid name"});

    int   argc   = 2;
    char* argv[] = {"filename", "invalid"};

    local_parser.parse(argc, argv);
    auto value = local_parser.get_value("invalid");
    EXPECT_FALSE(value.has_value());
}
