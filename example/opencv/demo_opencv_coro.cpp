#include <chrono>
#include <coroutine>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
constexpr const char* PIC_PATH = R"(C:\Users\wddjwk\Pictures\Saved Pictures)";

// 用于存储图像信息的结构体
struct ImageInfo {
  std::string path;
  cv::Size size;
  double aspectRatio;
};

// 单线程实现
std::vector<ImageInfo> processImagesSequential() {
  std::vector<ImageInfo> results;

  // 检查目录是否存在
  if (!fs::exists(PIC_PATH) || !fs::is_directory(PIC_PATH)) {
    std::cerr << "The specified path does not exist or is not a directory!" << '\n';
    return results;
  }

  // 遍历目录中的所有文件
  for (const auto& entry : fs::directory_iterator(PIC_PATH)) {
    if (entry.is_regular_file()) {
      const std::string ext = entry.path().extension().string();
      // 检查是否为图像文件
      if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" || ext == ".tif" || ext == ".tiff"
          || ext == ".webp" || ext == ".JPG" || ext == ".JPEG" || ext == ".PNG") {
        const std::string filePath = entry.path().string();

        // 读取图像尺寸而不加载整个图像到内存
        cv::Mat img = cv::imread(filePath);
        if (!img.empty()) {
          ImageInfo info;
          info.path = filePath;
          info.size = img.size();
          info.aspectRatio = static_cast<double>(info.size.width) / info.size.height;
          results.push_back(info);
        }
      }
    }
  }

  return results;
}

// 多线程实现
class ThreadedImageProcessor {
  private:
  std::vector<ImageInfo> results;
  std::mutex resultsMutex;
  std::vector<std::string> imagePaths;

  void processImage(const std::string& path) {
    cv::Mat img = cv::imread(path);
    if (!img.empty()) {
      ImageInfo info;
      info.path = path;
      info.size = img.size();
      info.aspectRatio = static_cast<double>(info.size.width) / info.size.height;

      std::lock_guard<std::mutex> lock(resultsMutex);
      results.push_back(info);
    }
  }

  public:
  std::vector<ImageInfo> processImages(int numThreads = std::thread::hardware_concurrency()) {
    // 收集所有图像路径
    if (!fs::exists(PIC_PATH) || !fs::is_directory(PIC_PATH)) {
      std::cerr << "The specified path does not exist or is not a directory!" << '\n';
      return results;
    }

    for (const auto& entry : fs::directory_iterator(PIC_PATH)) {
      if (entry.is_regular_file()) {
        const std::string ext = entry.path().extension().string();
        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" || ext == ".tif" || ext == ".tiff"
            || ext == ".webp" || ext == ".JPG" || ext == ".JPEG" || ext == ".PNG") {
          imagePaths.push_back(entry.path().string());
        }
      }
    }

    // 创建线程池
    std::vector<std::thread> threads;
    const size_t imagesPerThread = (imagePaths.size() + numThreads - 1) / numThreads;

    for (size_t t = 0; t < numThreads && t * imagesPerThread < imagePaths.size(); ++t) {
      threads.emplace_back([this, t, imagesPerThread]() {
        const size_t start = t * imagesPerThread;
        const size_t end = std::min(start + imagesPerThread, imagePaths.size());

        for (size_t i = start; i < end; ++i) {
          processImage(imagePaths[i]);
        }
      });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
      if (thread.joinable()) {
        thread.join();
      }
    }

    return results;
  }
};

// C++20 协程实现
struct ImageTask {
  struct promise_type {
    ImageInfo result;

    ImageTask get_return_object() { return ImageTask{std::coroutine_handle<promise_type>::from_promise(*this)}; }

    std::suspend_never initial_suspend() { return {}; }

    std::suspend_always final_suspend() noexcept { return {}; }

    void unhandled_exception() {}

    void return_value(ImageInfo value) { result = std::move(value); }
  };

  std::coroutine_handle<promise_type> handle;

  ImageInfo getResult() const { return handle.promise().result; }

  // Properly manage the handle to avoid double-free or invalid access
  explicit ImageTask(std::coroutine_handle<promise_type> h) : handle(h) {}

  // Move constructor
  ImageTask(ImageTask&& other) noexcept : handle(other.handle) { other.handle = nullptr; }

  // Move assignment
  ImageTask& operator=(ImageTask&& other) noexcept {
    if (this != &other) {
      if (handle) {
        handle.destroy();
      }
      handle = other.handle;
      other.handle = nullptr;
    }
    return *this;
  }

  // Disable copying
  ImageTask(const ImageTask&) = delete;
  ImageTask& operator=(const ImageTask&) = delete;

  ~ImageTask() {
    if (handle) {
      handle.destroy();
    }
  }
};

ImageTask processImageCoroutine(std::string path) {
  cv::Mat img = cv::imread(path);

  if (!img.empty()) {
    ImageInfo info;
    info.path = path;
    info.size = img.size();
    info.aspectRatio = static_cast<double>(info.size.width) / info.size.height;
    co_return info;
  }

  co_return ImageInfo{.path = path, .size = cv::Size(0, 0), .aspectRatio = 0.0};
}

std::vector<ImageInfo> processImagesWithCoroutines() {
  std::vector<ImageInfo> results;
  std::vector<std::unique_ptr<ImageTask>> tasks;

  // Check if directory exists
  if (!fs::exists(PIC_PATH) || !fs::is_directory(PIC_PATH)) {
    std::cerr << "The specified path does not exist or is not a directory!" << '\n';
    return results;
  }

  // Create coroutine tasks
  for (const auto& entry : fs::directory_iterator(PIC_PATH)) {
    if (entry.is_regular_file()) {
      const std::string ext = entry.path().extension().string();
      if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" || ext == ".tif" || ext == ".tiff"
          || ext == ".webp" || ext == ".JPG" || ext == ".JPEG" || ext == ".PNG") {
        // Use unique_ptr to manage ImageTask lifetime
        tasks.push_back(std::make_unique<ImageTask>(processImageCoroutine(entry.path().string())));
      }
    }
  }

  // Collect results
  for (const auto& task : tasks) {
    ImageInfo info = task->getResult();
    if (info.size.width > 0 && info.size.height > 0) {
      results.push_back(info);
    }
  }

  return results;
}

// 打印结果辅助函数
void printResults(const std::vector<ImageInfo>& results) {
  std::cout << "Processed " << results.size() << " images:" << '\n';

  // Only print details for the first 5 images
  for (size_t i = 0; i < std::min(results.size(), size_t(5)); ++i) {
    const auto& info = results[i];
    std::cout << "Image " << i + 1 << ": " << fs::path(info.path).filename().string() << '\n';
    std::cout << "  Size: " << info.size.width << " x " << info.size.height << '\n';
    std::cout << "  Aspect Ratio: " << info.aspectRatio << '\n';
  }

  if (results.size() > 5) {
    std::cout << "... Details for the remaining " << results.size() - 5 << " images omitted ..." << '\n';
  }
}

int main() {
  // 1. Sequential version
  std::cout << "Running sequential version..." << '\n';
  auto start = std::chrono::high_resolution_clock::now();
  auto sequentialResults = processImagesSequential();
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> sequentialTime = end - start;
  std::cout << "Sequential processing time: " << sequentialTime.count() << " seconds" << '\n';
  printResults(sequentialResults);
  std::cout << '\n';

  // 2. Multi-threaded version
  std::cout << "Running multi-threaded version..." << '\n';
  ThreadedImageProcessor processor;
  start = std::chrono::high_resolution_clock::now();
  auto threadedResults = processor.processImages();
  end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> threadedTime = end - start;
  std::cout << "Multi-threaded processing time: " << threadedTime.count() << " seconds" << '\n';
  std::cout << "Speedup vs sequential: " << sequentialTime.count() / threadedTime.count() << '\n';
  printResults(threadedResults);
  std::cout << '\n';

  // 3. Coroutine version
  std::cout << "Running coroutine version..." << '\n';
  start = std::chrono::high_resolution_clock::now();
  auto coroutineResults = processImagesWithCoroutines();
  end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> coroutineTime = end - start;
  std::cout << "Coroutine processing time: " << coroutineTime.count() << " seconds" << '\n';
  std::cout << "Speedup vs sequential: " << sequentialTime.count() / coroutineTime.count() << '\n';
  std::cout << "Speed ratio vs multi-threaded: " << threadedTime.count() / coroutineTime.count() << '\n';
  printResults(coroutineResults);

  return 0;
}