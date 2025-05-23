#ifndef SHUAIKAI_UTILS_NET_TOOLS_H
#define SHUAIKAI_UTILS_NET_TOOLS_H

#include <curl/curl.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <string>

#include "logger.h"
#include "macro.h"
#include "string_utils.h"

namespace fs = std::filesystem;

class AsyncDownloader {
public:
    enum class STATUS {
        CURL_FAILED,
        HTTP_INFO,
        HTTP_SUCCESS,
        HTTP_REDIRECTION,
        HTTP_CLIENT_ERROR,
        HTTP_SERVER_ERROR,
        UNKNOWN
    };

    static STATUS CheckHttpStatusCode(std::int64_t status_code) {
        if (status_code < 0) {
            return STATUS::CURL_FAILED;
        }
        switch (status_code / 100) {
            case 1: return STATUS::HTTP_INFO;
            case 2: return STATUS::HTTP_SUCCESS;
            case 3: return STATUS::HTTP_REDIRECTION;
            case 4: return STATUS::HTTP_CLIENT_ERROR;
            case 5: return STATUS::HTTP_SERVER_ERROR;
            default: return STATUS::UNKNOWN;
        }
    }

    // 下载到内存（返回字符串）
    static std::future<std::pair<std::string, std::int64_t>> FetchAsString(const std::string& url) {
        return std::async(std::launch::async, [url]() {
            CURL* curl = curl_easy_init();
            if (!curl) {
                return std::pair<std::string, std::int64_t>{"CURL_INIT_FAILED", CURL_INIT_FAIL_CODE};
            }

            std::string response;
            SetupCommonOptions(curl, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteStringCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            auto ret_code = ExecuteRequest(curl);
            curl_easy_cleanup(curl);

            return std::pair<std::string, std::int64_t>{response, ret_code};
        });
    }

    // 下载到本地文件
    static std::future<std::pair<bool, std::int64_t>> DownloadToFile(const std::string& url, const fs::path& filepath) {
        return std::async(std::launch::async, [url, filepath]() {
            CURL* curl    = curl_easy_init();
            bool  success = false;
            if (!curl) {
                return std::pair<bool, std::int64_t>{false, CURL_INIT_FAIL_CODE};
            }

            auto          ensured_filepath = EnsuredFilePath(url, filepath);
            std::ofstream file(ensured_filepath, std::ios::binary);
            if (!file.is_open()) {
                return std::pair<bool, std::int64_t>{false, INNER_FAIL_CODE};
            }

            SetupCommonOptions(curl, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

            auto ret_code = ExecuteRequest(curl);
            file.close();

            curl_easy_cleanup(curl);
            return std::pair<bool, std::int64_t>{(ret_code / 100 == 2), ret_code};
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
    static std::int64_t ExecuteRequest(CURL* curl) {
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            SK_ERROR("CURL error: {}", curl_easy_strerror(res));
            return CURL_EXEC_FAIL_CODE;
        }

        std::int64_t http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        return http_code;
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

    constexpr static std::int64_t CURL_INIT_FAIL_CODE = -1;
    constexpr static std::int64_t CURL_EXEC_FAIL_CODE = -2;
    constexpr static std::int64_t INNER_FAIL_CODE     = -3;
};

#endif  // SHUAIKAI_UTILS_NET_TOOLS_H
