#ifndef UNIX_SOCKET_HH
#define UNIX_SOCKET_HH
#include <array>
#include <string>
#include <unistd.h>
#include <vector>
#include <thread>
#include <mutex>


template <ssize_t BUFSIZE>
auto _fd_read(int file_descriptor) -> std::string {
    std::array<unsigned char, BUFSIZE> buffer{};
    ssize_t nbytes = read(file_descriptor, buffer.data(), std::size(buffer));
    return std::string(buffer.cbegin(), buffer.cbegin + nbytes - 1);
}


template <ssize_t BUFSIZE>
auto _fd_write(int file_descriptor, const std::string &msg) -> ssize_t {
    ssize_t nbytes = write(file_descriptor, msg.c_str(), std::size(msg));
    return nbytes;
}

class UnixSocket {
public:
    UnixSocket(std::string socket_path, int backlog);
    auto start() -> void;
    auto stop() -> void;

private:
    int _server_fd;
    int _backlog;
    std::string _socket_path;

    std::vector<int> _clients{};
    std::mutex _clients_mutex;

    std::atomic<bool> _is_listening = false;
    std::thread _server_thread;


    auto _add_client(int file_descriptor) -> void;
    auto _start_server() -> void;

    auto _handle_client() -> void;

};



#endif // UNIX_SOCKET_HH
