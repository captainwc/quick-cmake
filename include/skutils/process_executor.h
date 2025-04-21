#ifndef SK_UTILS_PROCESS_EXECUTOR_H
#define SK_UTILS_PROCESS_EXECUTOR_H

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>

#endif  // _WIN32

namespace sk::utils {

/**
 * 命令执行结果结构体
 */
struct ExecuteResult {
    int         exitCode;  // 进程退出码
    std::string output;    // 标准输出内容
    std::string error;     // 标准错误内容
    bool        success;   // 是否成功执行
};

class ProcessExecutor {
public:
    /**
     * 在当前系统执行命令并捕获输出
     *
     * @param executable 可执行文件路径
     * @param options 命令行参数
     * @param workingDir 工作目录（可选）
     * @param timeoutMs 超时时间（毫秒），默认为无限等待 (-1)
     * @return ExecuteResult 包含执行结果、输出和退出码的结构体
     */
    static ExecuteResult ExecuteCommand(const std::string& executable, const std::string& options,
                                        const std::string& workingDir = "", int timeoutMs = -1) {
#ifdef _WIN32
        return ExecuteCommandWindows(executable, options, workingDir,
                                     timeoutMs == -1 ? INFINITE : static_cast<DWORD>(timeoutMs));
#else
        return ExecuteCommandPosix(executable, options, workingDir, timeoutMs);
#endif
    }

private:
#ifdef _WIN32
    // Windows 平台实现
    static ExecuteResult ExecuteCommandWindows(const std::string& executable, const std::string& options,
                                               const std::string& workingDir, DWORD timeoutMs) {
        ExecuteResult result = {0, "", "", false};

        // 创建安全属性
        SECURITY_ATTRIBUTES securityAttributes;
        securityAttributes.nLength              = sizeof(SECURITY_ATTRIBUTES);
        securityAttributes.bInheritHandle       = TRUE;
        securityAttributes.lpSecurityDescriptor = NULL;

        // 创建管道用于标准输出
        HANDLE stdOutReadHandle = NULL, stdOutWriteHandle = NULL;
        if (!CreatePipe(&stdOutReadHandle, &stdOutWriteHandle, &securityAttributes, 0)) {
            result.error = "Failed to create stdout pipe. Error code: " + std::to_string(GetLastError());
            return result;
        }
        SetHandleInformation(stdOutReadHandle, HANDLE_FLAG_INHERIT, 0);

        // 创建管道用于标准错误
        HANDLE stdErrReadHandle = NULL, stdErrWriteHandle = NULL;
        if (!CreatePipe(&stdErrReadHandle, &stdErrWriteHandle, &securityAttributes, 0)) {
            CloseHandle(stdOutReadHandle);
            CloseHandle(stdOutWriteHandle);
            result.error = "Failed to create stderr pipe. Error code: " + std::to_string(GetLastError());
            return result;
        }
        SetHandleInformation(stdErrReadHandle, HANDLE_FLAG_INHERIT, 0);

        // 设置进程启动信息
        STARTUPINFOA startupInfo;
        ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);
        startupInfo.dwFlags |= STARTF_USESTDHANDLES;
        startupInfo.hStdOutput = stdOutWriteHandle;
        startupInfo.hStdError  = stdErrWriteHandle;
        startupInfo.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);

        // 准备命令行
        std::string             commandLine = executable + " " + options;
        std::unique_ptr<char[]> cmdLine(new char[commandLine.length() + 1]);
        strcpy_s(cmdLine.get(), commandLine.length() + 1, commandLine.c_str());

        // 准备工作目录
        LPCSTR lpWorkingDir = workingDir.empty() ? NULL : workingDir.c_str();

        // 创建进程
        PROCESS_INFORMATION processInfo;
        ZeroMemory(&processInfo, sizeof(processInfo));

        BOOL processCreated = CreateProcessA(NULL,              // 应用程序名称，使用NULL让系统从命令行中获取
                                             cmdLine.get(),     // 命令行参数
                                             NULL,              // 进程安全属性
                                             NULL,              // 线程安全属性
                                             TRUE,              // 继承句柄
                                             CREATE_NO_WINDOW,  // 创建标志，使用CREATE_NO_WINDOW不显示窗口
                                             NULL,              // 环境块
                                             lpWorkingDir,      // 工作目录
                                             &startupInfo,      // 启动信息
                                             &processInfo       // 进程信息
        );

        // 关闭管道写入端（子进程将使用）
        CloseHandle(stdOutWriteHandle);
        CloseHandle(stdErrWriteHandle);

        if (!processCreated) {
            CloseHandle(stdOutReadHandle);
            CloseHandle(stdErrReadHandle);
            result.error = "Failed to create process. Error code: " + std::to_string(GetLastError());
            return result;
        }

        result.success = true;

        // 读取标准输出和标准错误
        const int bufferSize = 4096;
        char      buffer[bufferSize];
        DWORD     bytesRead;

        // 读取标准输出
        while (true) {
            BOOL readSuccess = ReadFile(stdOutReadHandle, buffer, bufferSize - 1, &bytesRead, NULL);
            if (!readSuccess || bytesRead == 0)
                break;

            buffer[bytesRead] = '\0';
            result.output += buffer;
        }

        // 读取标准错误
        while (true) {
            BOOL readSuccess = ReadFile(stdErrReadHandle, buffer, bufferSize - 1, &bytesRead, NULL);
            if (!readSuccess || bytesRead == 0)
                break;

            buffer[bytesRead] = '\0';
            result.error += buffer;
        }

        // 等待进程完成，带有超时处理
        DWORD waitResult = WaitForSingleObject(processInfo.hProcess, timeoutMs);

        if (waitResult == WAIT_TIMEOUT) {
            // 终止进程
            TerminateProcess(processInfo.hProcess, 1);
            result.error += "Process execution timed out after " + std::to_string(timeoutMs) + " ms.";
            result.exitCode = -1;
        } else {
            // 获取退出码
            DWORD exitCode = 0;
            GetExitCodeProcess(processInfo.hProcess, &exitCode);
            result.exitCode = static_cast<int>(exitCode);
        }

        // 关闭所有句柄
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(stdOutReadHandle);
        CloseHandle(stdErrReadHandle);

        return result;
    }
#else
    // POSIX 平台实现（Linux/macOS）
    static ExecuteResult ExecuteCommandPosix(const std::string& executable, const std::string& options,
                                             const std::string& workingDir, int timeoutMs) {
        ExecuteResult result = {0, "", "", false};

        // 创建标准输出管道
        int stdoutPipe[2];
        if (pipe(stdoutPipe) != 0) {
            result.error = "Failed to create stdout pipe: " + std::string(strerror(errno));
            return result;
        }

        // 创建标准错误管道
        int stderrPipe[2];
        if (pipe(stderrPipe) != 0) {
            close(stdoutPipe[0]);
            close(stdoutPipe[1]);
            result.error = "Failed to create stderr pipe: " + std::string(strerror(errno));
            return result;
        }

        // 创建子进程
        pid_t pid = fork();

        if (pid < 0) {
            // Fork 失败
            close(stdoutPipe[0]);
            close(stdoutPipe[1]);
            close(stderrPipe[0]);
            close(stderrPipe[1]);
            result.error = "Failed to fork process: " + std::string(strerror(errno));
            return result;
        } else if (pid == 0) {
            // 子进程代码

            // 更改工作目录（如果指定）
            if (!workingDir.empty()) {
                if (chdir(workingDir.c_str()) != 0) {
                    _exit(1);  // 退出子进程
                }
            }

            // 重定向标准输出到管道
            dup2(stdoutPipe[1], STDOUT_FILENO);
            dup2(stderrPipe[1], STDERR_FILENO);

            // 关闭所有管道描述符
            close(stdoutPipe[0]);
            close(stdoutPipe[1]);
            close(stderrPipe[0]);
            close(stderrPipe[1]);

            // 准备命令和参数
            std::string fullCommand = executable + " " + options;
            execl("/bin/sh", "sh", "-c", fullCommand.c_str(), NULL);

            // 如果execl失败
            _exit(1);
        }

        // 父进程代码

        // 关闭管道写入端（子进程将使用）
        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        // 设置非阻塞标志
        fcntl(stdoutPipe[0], F_SETFL, O_NONBLOCK);
        fcntl(stderrPipe[0], F_SETFL, O_NONBLOCK);

        result.success = true;

        // 使用poll监听管道
        struct pollfd fds[2];
        fds[0].fd     = stdoutPipe[0];
        fds[0].events = POLLIN;
        fds[1].fd     = stderrPipe[0];
        fds[1].events = POLLIN;

        bool stdoutClosed = false;
        bool stderrClosed = false;
        char buffer[4096];

        // 计算结束时间
        long endTime = -1;
        if (timeoutMs > 0) {
            endTime = static_cast<long>(time(NULL)) * 1000 + timeoutMs;
        }

        // 读取输出直到子进程结束或超时
        while (!stdoutClosed || !stderrClosed) {
            // 计算剩余超时时间
            int remainingTimeMs = -1;
            if (timeoutMs > 0) {
                long currentTime = static_cast<long>(time(NULL)) * 1000;
                if (currentTime >= endTime) {
                    // 已超时
                    kill(pid, SIGTERM);
                    usleep(100000);      // 给进程一点时间终止
                    kill(pid, SIGKILL);  // 强制终止
                    result.error += "Process execution timed out after " + std::to_string(timeoutMs) + " ms.";
                    result.exitCode = -1;
                    break;
                }
                remainingTimeMs = static_cast<int>(endTime - currentTime);
            }

            // 等待数据可读
            int pollResult = poll(fds, 2, remainingTimeMs > 0 ? remainingTimeMs : 100);

            if (pollResult == -1) {
                if (errno == EINTR)
                    continue;  // 被信号中断，继续
                break;         // 错误
            }

            // 读取标准输出
            if (!stdoutClosed && (fds[0].revents & POLLIN)) {
                ssize_t bytesRead = read(stdoutPipe[0], buffer, sizeof(buffer) - 1);
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    result.output += buffer;
                } else if (bytesRead == 0 || (bytesRead == -1 && errno != EAGAIN)) {
                    stdoutClosed = true;
                }
            }

            // 读取标准错误
            if (!stderrClosed && (fds[1].revents & POLLIN)) {
                ssize_t bytesRead = read(stderrPipe[0], buffer, sizeof(buffer) - 1);
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    result.error += buffer;
                } else if (bytesRead == 0 || (bytesRead == -1 && errno != EAGAIN)) {
                    stderrClosed = true;
                }
            }

            // 检查管道是否已关闭
            if ((fds[0].revents & (POLLHUP | POLLERR)) && !(fds[0].revents & POLLIN)) {
                stdoutClosed = true;
            }
            if ((fds[1].revents & (POLLHUP | POLLERR)) && !(fds[1].revents & POLLIN)) {
                stderrClosed = true;
            }
        }

        // 关闭管道读取端
        close(stdoutPipe[0]);
        close(stderrPipe[0]);

        // 如果进程未因超时终止，获取退出状态
        if (result.exitCode != -1) {
            int status;
            waitpid(pid, &status, 0);

            if (WIFEXITED(status)) {
                result.exitCode = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                result.exitCode = 128 + WTERMSIG(status);
            } else {
                result.exitCode = 1;  // 未知状态
            }
        }

        return result;
    }
#endif
};

// 提供一个静态函数作为简单接口
inline ExecuteResult ExecuteCommand(const std::string& executable, const std::string& options,
                                    const std::string& workingDir = "", int timeoutMs = -1) {
    return ProcessExecutor::ExecuteCommand(executable, options, workingDir, timeoutMs);
}

}  // namespace sk::utils

#endif  // SK_UTILS_PROCESS_EXECUTOR_H
