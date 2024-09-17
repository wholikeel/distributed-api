#include <arpa/inet.h>
#include <csignal>
#include <expected>
#include <iostream>
#include <netdb.h>
#include <optional>
#include <peer.hh>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <addr_iter.hh>
#include <array>
#include <string>
#include <unistd.h>
#include <utility>

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

Peer::Peer(std::string_view port, unsigned int back_log)
    : _port(port), _back_log(back_log) {}

constexpr auto Peer::_get_hints(int ai_family, int ai_socktype, int ai_flags)
    -> struct addrinfo {
  struct addrinfo hints {
    .ai_flags = ai_flags, .ai_family = ai_family, .ai_socktype = ai_socktype,
  };
  return hints;
}

auto Peer::_start_server() {
  auto hints = _get_hints(AF_UNSPEC, SOCK_STREAM, AI_PASSIVE);

  auto server_info = get_addr_info("0.0.0.0", _port, hints);
  if (!server_info.has_value()) {
    std::cout << "getaddrinfo: " << gai_strerror(server_info.error()) << '\n';
  }
  auto can_reuse = 1; // TODO: should support N options
  auto address_data = get_valid_address(server_info.value(), &can_reuse);

  if (!address_data.has_value()) {
    std::cout << "Failed to bind\n";
    return 1;
  }

  auto sockfd = address_data.value();

  if (listen(sockfd, 10) == -1) {
    perror("listen");
    exit(1);
  }

  // Cull abandoned childred
  struct sigaction sig_action {};
  sig_action.sa_handler = sigchld_handler;
  sigemptyset(&sig_action.sa_mask);
  sig_action.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sig_action, nullptr) == -1) {
    perror("sigaction");
    return 1;
  }

  std::cout << "server: waiting for connections...\n";
  while (true) {
    auto conn = accept_conn(sockfd);
    if (!conn.has_value()) {
      continue;
    }
    auto new_addr = conn.value();
    auto new_fd = conn.value().first;
    auto their_addr = conn.value().second;

    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    auto addr_type = get_in_addr(&their_addr);
    if (!addr_type.has_value()) {
      std::cout << "Unknown address type\n";
      continue;
    }
    std::cout << "server: got connection from " << addr_type.value() << '\n';
    if (fork() == 0) {
      close(sockfd);
      if (send(new_fd, "Hello, world!\n") == -1) {
        perror("send");
      }
      close(new_fd);
      exit(0);
    }
    close(new_fd);
  }
  return 0;
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


  const int max_data_size = 100;
  std::array<char, max_data_size> buffer {0};
  ssize_t nbytes = 0;
  if ((nbytes = recv(sockfd, buffer.data(), max_data_size - 1, 0)) == -1) {
    perror("recv");
    exit(1);
  }
  buffer.at(nbytes) = '\0';

  close(sockfd);
}

auto Peer::start() -> void {
  auto ret = _start_server();
  std::cout << ret << '\n';
}
