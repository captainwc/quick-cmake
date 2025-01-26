#include <curl/curl.h>
#include <curl/easy.h>

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <ios>
#include <iterator>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "skutils/argparser.h"
#include "skutils/config.h"
#include "skutils/macro.h"
#include "skutils/net_tools.h"
#include "skutils/noncopyable.h"
#include "skutils/printer.h"
#include "skutils/string_utils.h"

using Json   = nlohmann::json;
namespace fs = std::filesystem;
namespace ks = sk::utils;

// TODO: Maybe can make it common
std::string ColorfulVector(const std::vector<std::string>& vc) {
    if (vc.empty()) {
        return "";
    }
    std::stringstream ss;
    ss << WITH_YELLOW("[");
    for (int i = 0; i < vc.size() - 1; ++i) {
        ss << WITH_BLUE(ks::toString(vc[i])) << WITH_YELLOW(", ");
    }
    ss << WITH_BLUE(ks::toString(vc[vc.size() - 1])) << WITH_YELLOW("]");
    return ss.str();
}

constexpr const char* WIN_SUFFIX[] = {"Windows_x86_64.zip",        "windows_amd64.zip",          "x86_64-windows.zip",
                                      "x86_64-pc-windows-gnu.zip", "x86_64-pc-windows-msvc.zip", "windows-amd64.zip",
                                      "Windows-msvc-x86_64.zip"};

constexpr const char* LINUX_SUFFIX[] = {
    "Linux_x86_64.tar.gz", "linux_x86_x64.tar.gz", "x86_64-linux.zip", "x86_64-unknown-linux-gnu.tar.gz",
    "linux_amd64.tar.gz",  "linux-amd64.tar.gz",   " amd64.deb ",      "Linux-gnu-x86_64.tar.gz"};

constexpr const char* USELESS_ABOUT = "";

std::map<std::string, std::string>& GetDefaultToolSet() {
    static std::map<std::string, std::string> TOOL_SET{
        {"bootandy/dust", "Graphically show folder size"},
        {"BurntSushi/ripgrep", "quicker `grep`"},
        {"Canop/broot", "Graphically and Interactivly `tree`"},
        {"dandavison/delta", "Colorful and syntax-highlighting `git-diff`"},
        {"eza-community/eza", "Modern `ls`, but i prefer `lsd`"},
        {"junegunn/fzf", "Fuzz find"},
        {"jesseduffield/lazygit", "Everyone who use git by command should have this."},
        {"lsd-rs/lsd", "Colorful `ls`"},
        {"muesli/duf", "Graphic `df`"},
        {"orf/gping", "Graphic `ping`"},
        {"sharkdp/bat", "syntax - highlighting `cat`"},
        {"sharkdp/fd", "quicker `find`"},
        {"sharkdp/hyperfine", "benchmark tool"},
        {"fastfetch-cli/fastfetch", "Replacement of `neofetch`"},
        {"nicolargo/glances", "One of many better `top`"},
        {"aristocratos/btop", "One of many better `top`"}};
    return TOOL_SET;
}

struct ToolInfo : public NonCopyable {
    std::string              repo;
    std::string              name;
    std::string              about;
    std::string              browser_link;
    std::string              api_address;
    std::vector<std::string> suffixes;

    std::future<std::pair<std::string, std::int64_t>>                 release_future;
    std::map<std::string, std::future<std::pair<bool, std::int64_t>>> download_links;

    explicit ToolInfo(const std::string& repo_name)
        : repo(repo_name),
          name(FromRepoToName(repo_name)),
          about(GetDefaultToolSet()[repo_name]),
          browser_link(GenerateBrowserLink(repo_name)),
          api_address(GenerateApiAddress(repo_name)) {
        if (sk::utils::IS_LINUX_OS()) {
            for (const auto* suffix : LINUX_SUFFIX) {
                suffixes.emplace_back(suffix);
            }
        } else {
            for (const auto* suffix : WIN_SUFFIX) {
                suffixes.emplace_back(suffix);
            }
        }
    }

    static std::string FromRepoToName(const std::string& repo) { return ks::str::split(repo, "/")[1]; }

    static std::string GenerateApiAddress(const std::string& repo) {
        return ks::format("https://api.github.com/repos/{}/releases/latest", ks::str::trim(ks::str::trim(repo, "\"")));
    }

    static std::string GenerateBrowserLink(const std::string& repo) {
        return ks::format("https://github.com/{}/releases", ks::str::trim(ks::str::trim(repo, "\"")));
    }

    static std::string ExtractFullReleaseNameFromUrl(const std::string& url) {
        return *(ks::str::split(url, "/").end() - 1);
    }
};

class DownloadTools {
public:
    explicit DownloadTools();

    void DownloadAll(const fs::path& directory);

    void Download(const std::vector<std::string>& tools, const fs::path& directory);

    void RegisterNewRepo(const std::string& repo);

    void ShowSupportedTools();

    void ShowLatestVersion();

private:
    static void AsyncFetchLatestReleaseInfo(std::vector<std::reference_wrapper<ToolInfo>>& tools);

    static void AsyncDownloadToDirectory(std::vector<std::reference_wrapper<ToolInfo>>& tools, const fs::path& dir);

    static void SyncAndExtractLinkFromRelease(ToolInfo& tool);

    static void SyncAndCheckDownloadTask(ToolInfo& tool, const fs::path& directory);

    static bool ExtraFileNameFilter(const std::string& github_release_name) {
        return !ks::str::contains(github_release_name, "musl");
    }

    std::vector<ToolInfo> tools_;

    // Just for a uniform api. Use tools_ cannot perform some filter on tool.name
    std::vector<std::reference_wrapper<ToolInfo>> tool_references_;
};

/// MARK: Implementations

DownloadTools::DownloadTools() {
    /** NOTE:
        The reason why the release information is not obtained asynchronously in the constructor
        is to prevent the information of all tools from being obtained
        while only individual tools are downloaded,
        which will occupy the number of GitHub requests.
    */
    auto& tool_set = GetDefaultToolSet();
    for (const auto& it : tool_set) {
        tools_.emplace_back(it.first);
    }
    std::copy(tools_.begin(), tools_.end(), std::back_inserter(tool_references_));
}

void DownloadTools::AsyncFetchLatestReleaseInfo(std::vector<std::reference_wrapper<ToolInfo>>& tools) {
    // (1) Async Get Release Information
    std::for_each(tools.begin(), tools.end(),
                  [](ToolInfo& tool) { tool.release_future = AsyncDownloader::FetchAsString(tool.api_address); });

    // (2) Sync Extract Download Link From Release Information
    std::for_each(tools.begin(), tools.end(), [](ToolInfo& tool) { SyncAndExtractLinkFromRelease(tool); });
}

void DownloadTools::AsyncDownloadToDirectory(std::vector<std::reference_wrapper<ToolInfo>>& tools,
                                             const fs::path&                                directory) {
    // (3) Async Start Download Task
    std::for_each(tools.begin(), tools.end(), [&directory](ToolInfo& tool) {
        for (auto& it : tool.download_links) {
            it.second = AsyncDownloader::DownloadToFile(it.first, directory);
        }
    });

    // (4) Sync And Check Download Task
    std::for_each(tools.begin(), tools.end(),
                  [&directory](ToolInfo& tool) { SyncAndCheckDownloadTask(tool, directory); });
}

void DownloadTools::SyncAndExtractLinkFromRelease(ToolInfo& tool) {
    auto [content, response_code] = tool.release_future.get();
    switch (AsyncDownloader::CheckHttpStatusCode(response_code)) {
        case AsyncDownloader::STATUS::HTTP_SUCCESS: {
            auto json = Json::parse(content);
            for (const auto& asset : json["assets"]) {
                for (const auto& suffix : tool.suffixes) {
                    auto release_name = asset.at("name").get<std::string>();
                    if (ks::str::endWith(release_name, suffix) && ExtraFileNameFilter(release_name)) {
                        tool.download_links.emplace(asset.at("browser_download_url"),
                                                    std::future<std::pair<bool, std::int64_t>>{});
                    }
                }
            }
            break;
        }
        case AsyncDownloader::STATUS::HTTP_CLIENT_ERROR: {
            ks::print("{}:{}", WITH_RED(ks::format("[ERROR-{}]", response_code)),
                      ks::format("{} {}.", WITH_BLUE("Failed to fetch release info of"), WITH_PURPLE(tool.name)));
            auto json = Json::parse(content);
            if (json.contains("message")) {
                ks::print(" [message]({}) [doucument_url]({})", WITH_GRAY(json.at("message")),
                          WITH_GRAY(json.at("documentation_url")));
            }
            std::cout << "\n";
            break;
        }
        default:
            ks::println("{}:{}", WITH_RED(ks::format("[ERROR-{}]", response_code)),
                        ks::format("{} {}.", WITH_BLUE("Failed to fetch release info of"), WITH_PURPLE(tool.name)));
    }
}

void DownloadTools::SyncAndCheckDownloadTask(ToolInfo& tool, const fs::path& directory) {
    for (auto& it : tool.download_links) {
        auto [flag, code] = it.second.get();
        if (flag) {
            ks::println("{}[{}] Download {} => {}", WITH_GREEN("[SUCCESS]"), WITH_PURPLE(tool.name),
                        WITH_YELLOW(ToolInfo::ExtractFullReleaseNameFromUrl(it.first)), WITH_BLUE(directory.string()));
        } else {
            ks::println("{}[{}] Failed Download {}.", WITH_RED(ks::format("[FAILURE-{}]", code)),
                        WITH_PURPLE(tool.name), WITH_BLUE(ToolInfo::ExtractFullReleaseNameFromUrl(it.first)));
        }
    }
}

void DownloadTools::RegisterNewRepo(const std::string& repo) {
    ToolInfo new_tool(repo);
    new_tool.release_future = AsyncDownloader::FetchAsString(new_tool.api_address);
    SyncAndExtractLinkFromRelease(new_tool);
    tools_.emplace_back(std::move(new_tool));
}

void DownloadTools::DownloadAll(const fs::path& directory) {
    AsyncFetchLatestReleaseInfo(tool_references_);
    AsyncDownloadToDirectory(tool_references_, directory);
}

void DownloadTools::Download(const std::vector<std::string>& tools, const fs::path& directory) {
    std::vector<std::reference_wrapper<ToolInfo>> filtered_tools;
    std::copy_if(tools_.begin(), tools_.end(), std::back_inserter(filtered_tools),
                 [&tools](auto& tool) { return std::find(tools.begin(), tools.end(), tool.name) != tools.end(); });

    AsyncFetchLatestReleaseInfo(filtered_tools);
    AsyncDownloadToDirectory(filtered_tools, directory);
}

void DownloadTools::ShowSupportedTools() {
    for (const auto& tool : tools_) {
        ks::println("[{}] {} => {}", WITH_PURPLE(tool.name), WITH_YELLOW(tool.about), WITH_BLUE(tool.browser_link));
    }
}

void DownloadTools::ShowLatestVersion() {
    AsyncFetchLatestReleaseInfo(tool_references_);
    for (const auto& tool : tools_) {
        std::vector<std::string> versions;
        for (const auto& it : tool.download_links) {
            versions.emplace_back(ToolInfo::ExtractFullReleaseNameFromUrl(it.first));  // NOLINT
        }
        ks::println("[{}] => {}", WITH_PURPLE(tool.repo), ColorfulVector(versions));
    }
}

/// MARK: Main

int main(int argc, char** argv) {
    ks::arg::ArgParser parser;
    parser.add_arg({"--check-update", ks::arg::ArgType::BOOL, .help = "Query Latest Version of Supported Tools"})
        .add_arg({"-c", ks::arg::ArgType::BOOL, .help = "Query Latest Version of Supported Tools"})
        .add_arg({"--list", ks::arg::ArgType::BOOL, .help = "List Supported Tools"})
        .add_arg({"-l", ks::arg::ArgType::BOOL, .help = "List Supported Tools"})
        .add_arg({"--output", ks::arg::ArgType::STR, .help = "OutPut Filename / Directory"})
        .add_arg({"-o", ks::arg::ArgType::STR, .help = "OutPut Filename / Directory"})
        .add_arg({"--tool", ks::arg::ArgType::LIST,
                  .help = R""([default] tools to dowload. `moderntools <fd>` or `moderntools <sharkdp/bat>`)""});

    parser.parse(argc, argv);

    if (parser.need_help()) {
        parser.show_help();
        return 0;
    }

    if (parser.get_value("-l").has_value() || parser.get_value("--list").has_value()) {
        DownloadTools().ShowSupportedTools();
        return 0;
    }

    if (parser.get_value("-c").has_value() || parser.get_value("--check-update").has_value()) {
        DownloadTools().ShowLatestVersion();
        return 0;
    }

    fs::path output_path;
    if (parser.get_value("-o").has_value()) {
        output_path = std::get<std::string>(parser.get_value("-o").value());  // NOLINT
    } else {
        output_path = std::get<std::string>(parser.get_value("--output").value_or(fs::current_path().string()));
    }

    auto downloader = DownloadTools();
    auto tools      = parser.get_value_with_default("--tool").value_or(std::vector<std::string>{});
    if (tools.empty()) {
        downloader.DownloadAll(output_path);
    } else {
        std::for_each(tools.begin(), tools.end(), [&downloader](auto& tool) {
            if (ks::str::contains(tool, "/")) {
                downloader.RegisterNewRepo(tool);
                tool = ToolInfo::FromRepoToName(tool);
            }
        });
        downloader.Download(tools, output_path);
    }
}
