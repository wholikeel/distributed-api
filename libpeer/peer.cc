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

// constexpr auto send(int file_descriptor, std::string_view message,
//                     int flags = 0) {
//   return send(file_descriptor, message.data(), message.length(), flags);
// }
//
// constexpr auto get_in_addr(struct sockaddr_storage *storage)
//     -> std::expected<std::string, int> {
//   auto *sockaddr = std::bit_cast<struct sockaddr *>(storage);
//   std::array<char, INET6_ADDRSTRLEN> net_buffer{};
//   switch (sockaddr->sa_family) {
//   case AF_INET:
//     inet_ntop(storage->ss_family,
//               &std::bit_cast<sockaddr_in *>(sockaddr)->sin_addr,
//               net_buffer.data(), net_buffer.size());
//     break;
//   case AF_INET6:
//     inet_ntop(storage->ss_family,
//               &std::bit_cast<sockaddr_in6 *>(sockaddr)->sin6_addr,
//               net_buffer.data(), net_buffer.size());
//     break;
//   default:
//     return std::unexpected(sockaddr->sa_family);
//   }
//   return std::string(net_buffer.cbegin(), net_buffer.cend());
// }
//
// auto accept_conn(int sockfd)
//     -> std::optional<std::pair<int, struct sockaddr_storage>> {
//   struct sockaddr_storage their_addr {};
//   socklen_t sin_size = sizeof(their_addr);
//   auto new_fd =
//       accept(sockfd, std::bit_cast<struct sockaddr *>(&their_addr), &sin_size);
//   if (new_fd == -1) {
//     std::cout << "accept returned -1\n";
//     return std::nullopt;
//   }
//   return std::make_pair(new_fd, their_addr);
// }
//
// constexpr auto get_addr_info(std::string_view port, struct addrinfo hints)
//     -> std::expected<struct addrinfo *, int> {
//   struct addrinfo *servinfo = nullptr;
//   int status = getaddrinfo(nullptr, port.data(), &hints, &servinfo);
//   if (status != 0) {
//     return std::unexpected(status);
//   }
//   return servinfo;
// }
//
// constexpr auto create_socket(struct addrinfo *sockaddr)
//     -> std::expected<int, SockError> {
//   auto sockfd =
//       socket(sockaddr->ai_family, sockaddr->ai_socktype, sockaddr->ai_protocol);
//   if (sockfd == -1) {
//     return std::unexpected(SockError::CREATION);
//   }
//   return sockfd;
// }
//
// template <typename T>
// constexpr auto set_sock_opt(int sockfd, int level, int optname, T *value)
//     -> std::expected<int, SockError> {
//   auto status = setsockopt(sockfd, level, optname, value, sizeof(T));
//   if (status == -1) {
//     return std::unexpected(SockError::SET_OPTION);
//   }
//   return sockfd;
// }
//
// auto bind_fd(int sockfd, const struct addrinfo *info)
//     -> std::expected<int, SockError> {
//   auto status = bind(sockfd, info->ai_addr, info->ai_addrlen);
//   if (status == -1) {
//     return std::unexpected(SockError::BIND);
//   }
//   return sockfd;
// }
//
// // TODO: needs refactor
// auto get_valid_address(struct addrinfo *addrinfo, int *can_reuse)
//     -> std::expected<int, SockError> {
//   for (auto *proto = addrinfo; proto != nullptr; proto = proto->ai_next) {
//     auto sockfd = create_socket(proto);
//     if (!sockfd.has_value()) {
//       freeaddrinfo(addrinfo);
//       return std::unexpected(SockError::CREATION);
//     }
//     auto configured_fd =
//         set_sock_opt(sockfd.value(), SOL_SOCKET, SO_REUSEADDR, can_reuse);
//     if (!configured_fd.has_value()) {
//       freeaddrinfo(addrinfo);
//       return std::unexpected(SockError::SET_OPTION);
//     };
//     auto bound_fd = bind_fd(sockfd.value(), proto);
//
//     if (bound_fd.has_value()) {
//       freeaddrinfo(addrinfo);
//       return sockfd.value();
//     }
//     close(sockfd.value());
//     break;
//   }
//   freeaddrinfo(addrinfo);
//   return std::unexpected(SockError::NONE_VALID);
// }

Peer::Peer(std::string_view port, unsigned int back_log)
    : _port(port), _back_log(back_log) {}

constexpr auto Peer::_get_hints(int ai_family, int ai_socktype, int ai_flags)
    -> struct addrinfo {
  struct addrinfo hints {
    .ai_flags = ai_flags, .ai_family = ai_family, .ai_socktype = ai_socktype,
  };
  return hints;
}

auto Peer::_start_listening() {
  auto hints = _get_hints(AF_UNSPEC, SOCK_STREAM, AI_PASSIVE);

  auto server_info = get_addr_info(_port, hints);
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

auto Peer::start() -> void {
  auto ret = _start_listening();
  std::cout << ret << '\n';
}
