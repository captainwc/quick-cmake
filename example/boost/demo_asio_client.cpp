#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <thread>

using boost::asio::ip::tcp;

int main() {
  try {
    // 创建 io_context 对象
    boost::asio::io_context io_context;

    // 创建并打开套接字
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "12345");
    tcp::socket socket(io_context);
    int i = 100;
    while (i--) {
      boost::asio::connect(socket, endpoints);

      std::cout << "Connected to server at 127.0.0.1:12345" << std::endl;

      // 发送数据
      const std::string request = "Hello from client!";
      boost::asio::write(socket, boost::asio::buffer(request));

      std::cout << "Sent: " << request << std::endl;

      // 接收响应
      char data[1024];
      size_t length = socket.read_some(boost::asio::buffer(data));
      std::string response(data, length);

      std::cout << "Received: " << response << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
