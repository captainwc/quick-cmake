#ifndef SHUAIKAI_UTILS_NET_TOOLS_H
#define SHUAIKAI_UTILS_NET_TOOLS_H

#include <curl/curl.h>

#include <filesystem>
#include <fstream>
#include <future>
#include <string>

#include "skutils/macro.h"
#include "skutils/string_utils.h"

namespace fs = std::filesystem;

class AsyncCurlDownloader {
public:
    // 下载到内存（返回字符串）
    static std::future<std::string> FetchToString(const std::string& url) {
        return std::async(std::launch::async, [url]() {
            CURL*       curl = curl_easy_init();
            std::string response;
            if (curl) {
                SetupCommonOptions(curl, url);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteStringCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
                ExecuteRequest(curl);
                curl_easy_cleanup(curl);
            }
            return response;
        });
    }

    // 下载到本地文件
    static std::future<bool> DownloadToFileAsync(const std::string& url, const fs::path& filepath) {
        return std::async(std::launch::async, [url, filepath]() {
            CURL* curl    = curl_easy_init();
            bool  success = false;
            if (!curl) {
                return false;
            }

            auto          ensured_filepath = EnsuredFilePath(url, filepath);
            std::ofstream file(ensured_filepath, std::ios::binary);
            if (!file.is_open()) {
                return false;
            }

            SetupCommonOptions(curl, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

            success = ExecuteRequest(curl);
            file.close();

            curl_easy_cleanup(curl);
            return success;
        });
    }

private:
    // 公共配置选项
    static void SetupCommonOptions(CURL* curl, const std::string& url) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");  // Any
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);        // 自动跟随 HTTP 3xx 重定向
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);       // 仅限制连接时间， 30s
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);             // 整体超时 300 秒
    }

    // 执行请求并处理错误
    static bool ExecuteRequest(CURL* curl) {
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            SK_ERROR("CURL error: {}", curl_easy_strerror(res));
            return false;
        }
        return true;
    }

    // 写入内存的回调函数
    static size_t WriteStringCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t total = size * nmemb;
        output->append(static_cast<char*>(contents), total);
        return total;
    }

    // 写入文件的回调函数
    static size_t WriteFileCallback(void* contents, size_t size, size_t nmemb, std::ofstream* file) {
        size_t total = size * nmemb;
        file->write(static_cast<char*>(contents), total);
        return total;
    }

    static fs::path EnsuredFilePath(const std::string& url, const fs::path& filepath) {
        bool is_directory = false;
        if (fs::exists(filepath)) {
            is_directory = fs::is_directory(filepath);
        } else {
            is_directory = !filepath.has_extension();
        }

        fs::path final_path;
        if (is_directory) {
            fs::create_directories(filepath);
            final_path = filepath / ExtractFilenameFromUrl(url);
        } else {
            fs::create_directories(filepath.parent_path());
            final_path = filepath;
        }
        return final_path;
    }

    static std::string ExtractFilenameFromUrl(const std::string& url) {
        return *(sk::utils::str::split(url, "/").end() - 1);
    }
};

#endif  // SHUAIKAI_UTILS_NET_TOOLS_H
