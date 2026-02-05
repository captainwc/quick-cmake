#include <iostream>
#include <string>

#include "skutils/event_manager.h"

using namespace sk::utils;
using namespace std;

// --- 用户自定义的简单数据结构 (无需继承任何基类) ---
struct LoginEvent {
    std::string username;
    int         userId;
};

struct DataUploadEvent {
    std::vector<float> data;
};

int main() {
    auto& bus = EventBus::GetInstance();

    // 1. 订阅 LoginEvent
    bus.Subscribe<LoginEvent>([](const LoginEvent& e) {
        std::cout << "[Sync] User logged in: " << e.username << " (ID: " << e.userId << ")" << std::endl;
    });

    // 2. 订阅 DataUploadEvent (模拟耗时操作)
    bus.Subscribe<DataUploadEvent>([](const DataUploadEvent& e) {
        std::cout << "[Async] Processing " << e.data.size() << " data points on thread " << std::this_thread::get_id()
                  << std::endl;
        // 模拟耗时
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    std::cout << "--- Start Publishing ---" << std::endl;

    // A. 同步发布：主线程直接调用
    bus.Publish(LoginEvent{"Alice", 1001});

    // B. 异步发布：丢给线程池，立即返回
    bus.PublishAsync(DataUploadEvent{{1.1f, 2.2f, 3.3f}});

    // C. 延时发布：2秒后执行 (实现定时/超时效果)
    std::cout << "Scheduling delayed task (2000ms)..." << std::endl;
    bus.PublishDelayed(LoginEvent{"Bob_Delayed", 9999}, 2000);

    // 主线程做点别的，防止直接退出
    std::cout << "Main thread continues work..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    return 0;
}
