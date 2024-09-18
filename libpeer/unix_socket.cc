#include <bit>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <syncstream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <utility>

#include "unix_socket.hh"

UnixSocket::UnixSocket(std::string socket_path, int backlog)
    : _server_fd(-1), _backlog(backlog), _socket_path(std::move(socket_path)) {}

UnixSocket::~UnixSocket() {
    stop();
}

auto UnixSocket::start() -> void {
  _server_thread = std::thread(&UnixSocket::_start_server, this);
}

auto UnixSocket::stop() -> void {
  _is_listening = false;
  _server_thread.join();
}

auto UnixSocket::_start_server() -> void {

  _server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (_server_fd == -1) {
    std::osyncstream(std::cerr)
        << "COULD NOT CREATE SOCKET: " << strerror(errno) << '\n';
    return;
  }

  struct sockaddr_un address {};

  address.sun_family = AF_UNIX;
  strncpy(address.sun_path, _socket_path.c_str(), _socket_path.length());

  unlink(_socket_path.c_str());

  if (bind(_server_fd, std::bit_cast<struct sockaddr *>(&address),
           sizeof(address)) == -1) {
    std::osyncstream(std::cerr)
        << "UNIXSOCKET BIND ERROR: " << strerror(errno) << '\n';
    close(_server_fd);
  }

  if (listen(_server_fd, _backlog) == -1) {
    std::osyncstream(std::cerr)
        << "COULD NOT LISTEN: " << strerror(errno) << '\n';
    close(_server_fd);
  }

  _is_listening = true;
  while (_is_listening) {
    // TODO: use poll syscall
    auto _client_fd = accept(_server_fd, nullptr, nullptr);
    if (_client_fd == -1) {
      std::osyncstream(std::osyncstream(std::cerr))
          << "ACCEPT FAILED: " << strerror(errno) << '\n';
      continue;
    }
    _add_client(_client_fd);
  }
  close(_server_fd);
  unlink(_socket_path.c_str());
}

auto UnixSocket::_add_client(int file_descriptor) -> void {
  std::lock_guard<std::mutex> lock(_clients_mutex);
  _clients.push_back(file_descriptor);
  std::osyncstream(std::cout) << "Added client: " << file_descriptor << '\n';
  auto nbytes = _fd_write<256>(file_descriptor, "hello from c++");
}
