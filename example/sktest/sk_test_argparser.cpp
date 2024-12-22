#include <string>
#include <vector>

#include "skutils/argparser.h"
#include "skutils/macro.h"
#include "skutils/printer.h"

int main() {
    namespace ag = sk::utils::arg;

    ag::ArgParser parser;
    parser.add_arg({.name = "--path", .type = ag::ArgType::LIST, .help = "File or Directory"})
        .add_arg({.name = "-o", .type = ag::ArgType::STR, .help = "Out Put Name"})
        .add_arg({.name = "-i", .type = ag::ArgType::INT, .help = "Short Path"})
        .add_arg({.name = "-bool", .type = ag::ArgType::BOOL, .help = "Boolean Value"})
        .add_arg({.name = "error", .type = ag::ArgType::STR, .help = "Skipped Value"});

    int         argc   = 14;
    const char* argv[] = {"filename", "default_val", "-o",    "false_outname", "-o =true_outname ",
                          "--path",   "path1",       "path2", "--path",        "dir/path3",
                          "-i",       "123",         "-bool", "unparsed",      "out_range"};

    parser.parse(argc, argv);

    auto parsed_filename      = parser.get_file_name();
    auto parsed_dafault_value = parser.get_default_args().value_or(std::vector<std::string>{""});
    auto parsed_out_name      = std::get<std::string>(parser.get_value("-o").value_or("FALSE_NAME"));
    auto parsed_path          = std::get<std::vector<std::string>>(
        parser.get_value("--path").value_or(std::vector<std::string>{"FALSE_VALUE"}));
    auto parsed_int  = std::get<int>(parser.get_value("-i").value_or(-1));
    auto parsed_bool = std::get<bool>(parser.get_value("-bool").value_or(false));

    auto correct_default_value = std::vector<std::string>{"default_val"};
    auto correct_path          = std::vector<std::string>{"path1", "path2", "dir/path3"};

    ASSERT_STR_EQUAL(parsed_filename, "filename");
    ASSERT_STR_EQUAL(parsed_dafault_value, correct_default_value);
    ASSERT_STR_EQUAL(parsed_out_name, "true_outname");
    ASSERT_STR_EQUAL(parsed_path, correct_path);
    ASSERT_EQUAL(parsed_int, 123);
    ASSERT_TRUE(parsed_bool);

    parser.help();

    return ASSERT_ALL_PASSED();
}