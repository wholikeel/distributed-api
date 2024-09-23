#include <memory>
#include <peer.hh>

#include "epoll_event_loop.hh"
#include "epoll_wrapper.hh"
#include "socket_wrapper.hh"

auto sigchld_handler(int pid) {
  int saved_errno = errno;
  while (waitpid(-1, nullptr, WNOHANG) > 0) {
  }
  errno = saved_errno;
}

// TODO: change to C++ style, merge with `socket_wrapper` vers
auto get_in_addr(struct sockaddr *sockaddr) -> void * {
  if (sockaddr->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sockaddr)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sockaddr)->sin6_addr);
}

Peer::Peer(std::string port, int back_log)
    : _port(std::move(port)), _back_log(back_log) {}

Peer::~Peer() { stop(); }

constexpr auto Peer::_get_hints(int ai_family, int ai_socktype, int ai_flags)
    -> struct addrinfo {
  struct addrinfo hints {
    .ai_flags = ai_flags, .ai_family = ai_family, .ai_socktype = ai_socktype,
  };
  return hints;
}


auto accept_tcp_connection() -> void {
    std::osyncstream(std::cout) << "client connected\n";
}

auto Peer::_start_server() -> void {
  if (_is_listening) {
    std::osyncstream(std::cerr) << "peer: cannot start, alreading listening\n";
  }

  _server_fd = create_tcp_listener("0.0.0.0", _port, _back_log);
  if (_server_fd == -1) {
    return;
  }

  // Cull abandoned childred
  struct sigaction sig_action {};
  sig_action.sa_handler = sigchld_handler;
  sigemptyset(&sig_action.sa_mask);
  sig_action.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sig_action, nullptr) == -1) {
    perror("sigaction");
    return;
  }

  if ((_epoll_fd = epoll_create1(EPOLL_CLOEXEC)) == -1) {
    return;
  }

  _event_loop = std::make_shared<EpollEventLoop>(_epoll_fd);
    

  auto *tcp = new TcpConnectionHandler(_event_loop, _server_fd);
  _event_loop->add_event_fd(_server_fd, tcp);

  _is_listening = true;
  while (_is_listening) {
    _event_loop->start(&_is_listening);
  }

  delete tcp;
}


auto Peer::_accept_client(int sock) -> std::optional<int> {
  auto client_fd = accept4_conn(sock, SOCK_CLOEXEC | SOCK_NONBLOCK);
  if (!client_fd.has_value()) {
    std::osyncstream(std::cerr) << "could not accept client\n";
    return std::nullopt;
  }
  return client_fd.value().first;
}

auto Peer::_client_connect(const std::string &address,
                           const std::string &port) {
  auto hints = _get_hints(AF_UNSPEC, SOCK_STREAM, 0);

  auto server_info = get_addr_info(address, port, hints);
  if (!server_info.has_value()) {
    std::cout << "getaddrinfo: " << gai_strerror(server_info.error()) << '\n';
  }

  int sockfd = -1;
  struct addrinfo *proto = nullptr;
  for (proto = server_info.value(); proto != nullptr; proto = proto->ai_next) {
    auto sock_resp = create_socket(proto);
    if (!sock_resp.has_value()) {
      perror("client: socket");
      continue;
    }
    sockfd = sock_resp.value();

    if (connect(sockfd, proto->ai_addr, proto->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }
    break;
  }

  if (proto == nullptr) {
    std::cout << "client: failed to connect\n";
    return;
  }

  std::array<char, INET6_ADDRSTRLEN> net_buffer{};
  inet_ntop(proto->ai_family, get_in_addr((struct sockaddr *)proto->ai_addr),
            net_buffer.data(), net_buffer.size());

  auto name = std::string(net_buffer.cbegin(), net_buffer.cend());

  std::cout << "client: connecting to " << name << '\n';

  freeaddrinfo(server_info.value());

  _routing_table.push_back(name);

  const int max_data_size = 100;
  std::array<char, max_data_size> buffer{0};
  ssize_t nbytes = 0;
  if ((nbytes = recv(sockfd, buffer.data(), max_data_size - 1, 0)) == -1) {
    perror("recv");
    exit(1);
  }
  buffer.at(nbytes) = '\0';

  close(sockfd);
}

auto Peer::_create_epoll(int socket_fd) const -> void {
  auto efd = epoll_create1(EPOLL_CLOEXEC);
  if (efd == -1) {
    std::osyncstream(std::cerr) << "epoll create error\n";
    return;
  }
}

auto Peer::_add_to_epoll(int client_fd) const -> void {
  struct epoll_event epoll_event {};
  epoll_event.events = EPOLLIN;
  epoll_event.data.fd = client_fd;
  if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &epoll_event) == -1) {
    std::osyncstream(std::cerr) << "epoll add error\n";
  }
};

auto Peer::_remove_from_epoll(int client_fd) const -> void {
  struct epoll_event epoll_event {};
  epoll_event.events = EPOLLIN;
  epoll_event.data.fd = client_fd;
  if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client_fd, &epoll_event) == -1) {
    std::osyncstream(std::cerr) << "epoll remove error\n";
  }
}

auto Peer::start() -> void {
  _server_thread = std::thread(&Peer::_start_server, this);
}

auto Peer::stop() -> void {
  _is_listening = false;
  _server_thread.join();
}

auto Peer::add_entry(int data) -> void { _data_entries.push_back(data); }

auto Peer::_update_hash() {}

auto Peer::_add_peer(const std::string &peer) -> void {
  _routing_table.push_back(peer);
}
