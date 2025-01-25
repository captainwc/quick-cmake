#include <curl/curl.h>
#include <curl/easy.h>

#include <cstddef>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <future>
#include <ios>
#include <map>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "skutils/argparser.h"
#include "skutils/config.h"
#include "skutils/file.h"
#include "skutils/macro.h"
#include "skutils/net_tools.h"
#include "skutils/printer.h"
#include "skutils/string_utils.h"

using Json   = nlohmann::json;
namespace fs = std::filesystem;
namespace ks = sk::utils;

inline constexpr const char* RAW_CONFIG =
    R""({ "modern-tools": [ { "name": "lazygit", "repo": "jesseduffield/lazygit", "description": "simple terminal UI for git commands", "linux-suffix": [ "Linux_x86_64.tar.gz" ], "windows-suffix": [ "Windows_x86_64.zip" ] }, { "name": "dust", "repo": "bootandy/dust", "description": "A more intuitive version of du in rust", "linux-suffix": [ "x86_64-unknown-linux-gnu.tar.gz" ], "windows-suffix": [ "x86_64-pc-windows-gnu.zip" ] }, { "name": "duf", "repo": "muesli/duf", "description": "Disk Usage/Free Utility - a better 'df' alternative", "linux-suffix": [ "linux_x86_x64.tar.gz", "linux_amd64.deb" ], "windows-suffix": [ "Windows_x86_64.zip" ] }, { "name": "fd", "repo": "sharkdp/fd", "description": "A simple, fast and user-friendly alternative to 'find'", "linux-suffix": [ "x86_64-unknown-linux-gnu.tar.gz", "amd64.deb" ], "windows-suffix": [ "x86_64-pc-windows-gnu.zip" ] }, { "name": "rg", "repo": "BurntSushi/ripgrep", "description": "ripgrep recursively searches directories for a regex pattern while respecting your gitignore", "linux-suffix": [ "amd64.deb" ], "windows-suffix": [ "x86_64-pc-windows-gnu.zip" ] }, { "name": "fzf", "repo": "junegunn/fzf", "description": "A command-line fuzzy finder", "linux-suffix": [ "linux_amd64.tar.gz" ], "windows-suffix": [ "windows_amd64.zip" ] }, { "name": "hyperfine", "repo": "sharkdp/hyperfine", "description": "A command-line benchmarking tool", "linux-suffix": [ "x86_64-unknown-linux-gnu.tar.gz", "amd64.deb" ], "windows-suffix": [ "x86_64-pc-windows-msvc.zip" ] } ] })"";

struct ToolInfo {
    std::string                              name;
    std::string                              about;
    std::string                              api_address;
    std::vector<std::string>                 suffixes;
    std::future<std::string>                 release_future;
    std::map<std::string, std::future<bool>> download_links;
};

void from_json(const Json& j, ToolInfo& t);

class DownloadTools {
public:
    explicit DownloadTools(fs::path config_file = fs::path{});

    void DownloadAll(const fs::path& directory);

    void Download(const std::vector<std::string>& tools, const fs::path& directory);

    void PrintConfig();

    void ShowSupportedTools();

private:
    Json ParseConfig();

    static std::string ExtractUrlBaseName(const std::string& url);

    static bool ExtraFileNameFilter(const std::string& github_release_name) {
        return !ks::str::contains(github_release_name, "musl");
    }

    std::vector<ToolInfo> tools_;

    fs::path config_file_;
};

/// MARK: Implementations

void from_json(const Json& j, ToolInfo& t) {
    j.at("name").get_to(t.name);
    j.at("description").get_to(t.about);
    t.api_address    = ks::format("https://api.github.com/repos/{}/releases/latest", ks::str::trim(j.at("repo"), "\""));
    t.release_future = AsyncCurlDownloader::FetchToString(t.api_address);
    if (sk::utils::IS_LINUX_OS()) {
        for (const auto& suffix : j.at("linux-suffix")) {
            t.suffixes.emplace_back(suffix);
        }
    } else {
        for (const auto& suffix : j.at("windows-suffix")) {
            t.suffixes.emplace_back(suffix);
        }
    }
    // will fill download_links seprately!
}

DownloadTools::DownloadTools(fs::path config_file) : config_file_(std::move(config_file)) {
    auto conf = ParseConfig();
    for (auto& item : conf["modern-tools"]) {
        tools_.emplace_back(item.get<ToolInfo>());
    }
    for (auto& tool : tools_) {
        auto f    = tool.release_future.get();
        auto json = Json::parse(f);
        if (!json.contains("assets")) {
            ks::print("{}:{}", WITH_RED("[ERROR]"), WITH_BLUE("Failed to fetch release info"));
            if (json.contains("message")) {
                ks::print(" [message]({}) [doucument_url]({})", WITH_GRAY(json.at("message")),
                          WITH_GRAY(json.at("documentation_url")));
            }
            std::cout << "\n";
        } else {
            for (const auto& asset : json["assets"]) {
                for (const auto& suffix : tool.suffixes) {
                    auto release_name = asset.at("name").get<std::string>();
                    if (ks::str::endWith(release_name, suffix) && ExtraFileNameFilter(release_name)) {
                        tool.download_links.emplace(asset.at("browser_download_url"), std::future<bool>{});
                    }
                }
            }
        }
    }
}

void DownloadTools::DownloadAll(const fs::path& directory) {
    for (auto& tool : tools_) {
        for (auto& it : tool.download_links) {
            it.second = AsyncCurlDownloader::DownloadToFileAsync(it.first, directory);
        }
    }
    for (auto& tool : tools_) {
        for (auto& it : tool.download_links) {
            if (it.second.get()) {
                ks::println("{}[{}] Download {} => {}", WITH_GREEN("[SUCCESS]"), WITH_PURPLE(tool.name),
                            WITH_BLUE(ExtractUrlBaseName(it.first)), WITH_YELLOW(directory.string()));
            } else {
                ks::println("{}[{}] Failed Download {}.", WITH_RED("[FALIURE]"), WITH_PURPLE(tool.name),
                            WITH_BLUE(ExtractUrlBaseName(it.first)));
            }
        }
    }
}

void DownloadTools::Download(const std::vector<std::string>& tools, const fs::path& directory) {
    for (auto& tool : tools_) {
        if (std::find(tools.begin(), tools.end(), tool.name) != tools.end()) {
            for (auto& it : tool.download_links) {
                it.second = AsyncCurlDownloader::DownloadToFileAsync(it.first, directory);
            }
        }
    }
    for (auto& tool : tools_) {
        if (std::find(tools.begin(), tools.end(), tool.name) != tools.end()) {
            for (auto& it : tool.download_links) {
                if (it.second.get()) {
                    ks::println("{}[{}] Download {} => {}", WITH_GREEN("[SUCCESS]"), WITH_PURPLE(tool.name),
                                WITH_BLUE(ExtractUrlBaseName(it.first)), WITH_YELLOW(directory.string()));
                } else {
                    ks::println("{}[{}] Failed Download {}.", WITH_RED("[FALIURE]"), WITH_PURPLE(tool.name),
                                WITH_BLUE(ExtractUrlBaseName(it.first)));
                }
            }
        }
    }
}

Json DownloadTools::ParseConfig() {
    if (config_file_.empty()) {
        return Json::parse(RAW_CONFIG);
    }

    std::ifstream file(config_file_);

    if (!file.is_open()) {
        SK_ERROR("Cannot open config file: {}", config_file_.string());
        return Json::parse(RAW_CONFIG);
    }

    try {
        Json json;
        file >> json;
        return json;
    } catch (const std::exception& e) {
        SK_ERROR("Parse Json Failed: {}", config_file_.string());
        return Json::parse(RAW_CONFIG);
    }
}

std::string DownloadTools::ExtractUrlBaseName(const std::string& url) {
    return *(ks::str::split(url, "/").end() - 1);
}

void DownloadTools::PrintConfig() {
    auto json = Json::parse(RAW_CONFIG);
    std::cout << json.dump() << std::endl;
}

void DownloadTools::ShowSupportedTools() {
    for (const auto& tool : tools_) {
        std::vector<std::string> versions;
        for (const auto& it : tool.download_links) {
            versions.emplace_back(ExtractUrlBaseName(it.first));
        }
        ks::println("[{}] => {} ({})", WITH_PURPLE(tool.name), WITH_BLUE(versions), WITH_GRAY(tool.about));
    }
}

/// MARK: Main

int main(int argc, char** argv) {
    ks::arg::ArgParser parser;
    parser.add_arg({"--show-config", ks::arg::ArgType::BOOL, .help = "Print Default Config To stdout"})
        .add_arg({"--config", ks::arg::ArgType::STR, .help = "Specify config File"})
        .add_arg({"-c", ks::arg::ArgType::STR, .help = "Specify config File"})
        .add_arg({"--list", ks::arg::ArgType::BOOL, .help = "List Supported Tools"})
        .add_arg({"-l", ks::arg::ArgType::BOOL, .help = "List Supported Tools"})
        .add_arg({"--output", ks::arg::ArgType::STR, .help = "OutPut Filename / Directory"})
        .add_arg({"-o", ks::arg::ArgType::STR, .help = "OutPut Filename / Directory"})
        .add_arg({"--tool", ks::arg::ArgType::LIST, .help = "[default] tools to dowload"});

    parser.parse(argc, argv);

    if (parser.need_help()) {
        parser.show_help();
        return 0;
    }

    if (parser.get_value("--show-config").has_value()) {
        DownloadTools().PrintConfig();
        return 0;
    }

    if (parser.get_value("--list").has_value() || parser.get_value("-l").has_value()) {
        DownloadTools().ShowSupportedTools();
        return 0;
    }

    fs::path config_file;
    fs::path output_path;

    if (parser.get_value("-c").has_value()) {
        config_file = std::get<std::string>(parser.get_value("-c").value());
    } else {
        config_file = std::get<std::string>(parser.get_value("--config").value_or(fs::path{}.string()));
    }

    if (parser.get_value("-o").has_value()) {
        output_path = std::get<std::string>(parser.get_value("-o").value());
    } else {
        output_path = std::get<std::string>(parser.get_value("--output").value_or(fs::current_path().string()));
    }

    auto tools = parser.get_value_with_default("--tool").value_or(std::vector<std::string>{});

    if (tools.empty()) {
        DownloadTools(config_file).DownloadAll(output_path);
    } else {
        DownloadTools(config_file).Download(tools, output_path);
    }
}
