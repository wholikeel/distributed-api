#ifndef PEER_HH
#define PEER_HH
#include <addr_iter.hh>
#include <arpa/inet.h>
#include <array>
#include <atomic>
#include <csignal>
#include <expected>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <optional>
#include <string>
#include <string_view>
#include <syncstream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

#include "event_handler.hh"
#include "event_loop.hh"
#include "tcp_handler.hh"

class Peer {

public:
  Peer(std::string port, int back_log);
  ~Peer();

  auto start() -> void;
  auto stop() -> void;

  auto add_entry(int data) -> void;

private:
  int _id = 0;
  std::string _port;
  const int _back_log;
  std::vector<int> _data_entries;

  int _server_fd = -1;
  int _epoll_fd = -1;
  std::shared_ptr<EventLoop> _event_loop;

  // std::string _data_hash;

  std::vector<std::string> _routing_table;
  std::mutex _routing_table_mutex;

  bool _is_listening = false;
  std::thread _server_thread;

  TcpConnectionHandler *_conn_handler = nullptr;

  auto _create_epoll(int socket_fd) const -> void;
  auto _add_to_epoll(int client_fd) const -> void;
  auto _remove_from_epoll(int client_fd) const -> void;

  auto _start_server() -> void;
  auto _accept_client(int sock) -> std::optional<int>;

  auto _client_connect(const std::string &address, const std::string &port);

  auto _update_hash();

  auto _add_peer(const std::string &peer) -> void;
  // auto _find_next(const std::string &peer) -> void;

  constexpr auto _get_hints(int ai_family, int ai_socktype, int ai_flags)
      -> struct addrinfo;
};

#endif // PEER_HH
