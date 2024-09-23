#include "tcp_handler.hh"
#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <syncstream>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

static constexpr auto bufsize = 1024;


EchoHandler::EchoHandler(std::shared_ptr<EventLoop> event_loop, int client_fd) : _client_fd{client_fd}, _ev(std::move(event_loop)) {}
auto EchoHandler::handler() -> void {
  auto buf = std::array<char, bufsize>();
  auto nbytes = recv(_client_fd, buf.data(), buf.size(), 0);
  if (nbytes == -1) {
      close(_client_fd);
  };
  auto msg = std::string(buf.data(), buf.data() + nbytes);
  std::osyncstream(std::cout) << "recv: " << msg << '\n';

  send(_client_fd, msg.data(), msg.length(), 0);

}


TcpConnectionHandler::TcpConnectionHandler(
    std::shared_ptr<EventLoop> event_loop, int server_fd)
    : _server_fd(server_fd), _ev(std::move(event_loop)) {}

// TODO: should only add connections to epoll
auto TcpConnectionHandler::handler() -> void {
  struct sockaddr addr {};
  socklen_t size = 0;
  auto conn = accept(_server_fd, &addr, &size);
  if (conn == -1) {
    return;
  }
  std::osyncstream(std::cout) << "Recieved TCP connection\n";
  auto *echo = new EchoHandler(_ev, conn);
  _ev->add_event_fd(conn, echo);
  // close(conn);
}
