#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main() {
    try {
        // 创建 io_context 对象
        boost::asio::io_context io_context;

        // 创建并打开套接字
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));

		std::cout << "Server Listen on " << "tcp::v4()" << ":12345 ...\n";
		std::cout << "Waiting for connection ...\n";

        for (;;) {
            // 等待客户端连接
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            std::cout << "Client connected!" << std::endl;

            // 读取数据
            char        data[1024];
            size_t      length = socket.read_some(boost::asio::buffer(data));
            std::string message(data, length);

            std::cout << "Received: " << message << std::endl;

            // 发送响应
            const std::string response = "Hello from server!";
            boost::asio::write(socket, boost::asio::buffer(response));

            std::cout << "Response sent." << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
