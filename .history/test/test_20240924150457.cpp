#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

using boost::asio::ip::tcp;

std::string get_process_list() {
    DIR* dir;
    struct dirent* entry;

    dir = opendir("/proc");
    if (dir == nullptr) {
        return "Ошибка при открытии /proc\n";
    }

    std::stringstream processes;
    processes << "<ul>";
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
            processes << "<li>PID: " << entry->d_name << "</li>";
        }
    }
    processes << "</ul>";
    closedir(dir);

    return processes.str();
}

std::string kill_process(int pid) {
    if (kill(pid, SIGKILL) == 0) {
        return "Процесс " + std::to_string(pid) + " успешно завершен.";
    } else {
        return "Не удалось завершить процесс " + std::to_string(pid);
    }
}

class HttpServer {
public:
    HttpServer(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        start_accept();
    }

private:
    void start_accept() {
        tcp::socket socket(acceptor_.get_io_context());
        acceptor_.async_accept(socket, 
            boost::bind(&HttpServer::handle_accept, this, 
                boost::asio::placeholders::error, std::move(socket)));
    }

    void handle_accept(const boost::system::error_code& error, tcp::socket socket) {
        if (!error) {
            std::array<char, 1024> buffer;
            boost::system::error_code read_error;
            size_t length = socket.read_some(boost::asio::buffer(buffer), read_error);
            if (read_error) {
                return;
            }

            std::string request(buffer.data(), length);
            std::string response;

            if (request.find("GET /processes") != std::string::npos) {
                response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
                response += get_process_list();
            } else if (request.find("GET /kill?pid=") != std::string::npos) {
                int pid = std::stoi(request.substr(request.find("pid=") + 4));
                response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
                response += kill_process(pid);
            } else {
                response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nInvalid request";
            }

            boost::asio::write(socket, boost::asio::buffer(response), read_error);
        }
        start_accept();
    }

    tcp::acceptor acceptor_;
};

int main() {
    try {
        boost::asio::io_context io_context;
        HttpServer server(io_context, 8080);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
    return 0;
}