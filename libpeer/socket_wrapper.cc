#include "socket_wrapper.hh"
#include <bit>
#include <netdb.h>
#include <netinet/in.h>
#include <syncstream>
#include <sys/socket.h>

auto send(int file_descriptor, std::string_view message, int flags) -> ssize_t {
  return send(file_descriptor, message.data(), message.length(), flags);
}

auto accept_conn(int sockfd)
    -> std::optional<std::pair<int, struct sockaddr_storage>> {
  struct sockaddr_storage their_addr {};
  socklen_t sin_size = sizeof(their_addr);
  auto new_fd =
      accept(sockfd, std::bit_cast<struct sockaddr *>(&their_addr), &sin_size);
  if (new_fd == -1) {
    return std::nullopt;
  }
  return std::make_pair(new_fd, their_addr);
}

auto accept4_conn(int sockfd, int flags)
    -> std::optional<std::pair<int, struct sockaddr_storage>> {
  struct sockaddr_storage their_addr {};
  socklen_t sin_size = sizeof(their_addr);
  auto new_fd = accept4(sockfd, std::bit_cast<struct sockaddr *>(&their_addr),
                        &sin_size, flags);
  if (new_fd == -1) {
    return std::nullopt;
  }
  return std::make_pair(new_fd, their_addr);
}

auto get_in_addr(struct sockaddr_storage *storage)
    -> std::expected<std::string, int> {
  auto *sockaddr = std::bit_cast<struct sockaddr *>(storage);
  std::array<char, INET6_ADDRSTRLEN> net_buffer{};
  switch (sockaddr->sa_family) {
  case AF_INET:
    inet_ntop(storage->ss_family,
              &std::bit_cast<sockaddr_in *>(sockaddr)->sin_addr,
              net_buffer.data(), net_buffer.size());
    break;
  case AF_INET6:
    inet_ntop(storage->ss_family,
              &std::bit_cast<sockaddr_in6 *>(sockaddr)->sin6_addr,
              net_buffer.data(), net_buffer.size());
    break;
  default:
    return std::unexpected(sockaddr->sa_family);
  }
  return std::string(net_buffer.cbegin(), net_buffer.cend());
}

auto get_addr_info(std::string_view hostname, std::string_view port,
                   struct addrinfo hints)
    -> std::expected<struct addrinfo *, int> {
  struct addrinfo *servinfo = nullptr;
  int status = getaddrinfo(hostname.data(), port.data(), &hints, &servinfo);
  if (status != 0) {
    return std::unexpected(status);
  }
  return servinfo;
}

auto create_socket(struct addrinfo *sockaddr) -> std::expected<int, SockError> {
  auto sockfd =
      socket(sockaddr->ai_family, sockaddr->ai_socktype, sockaddr->ai_protocol);
  if (sockfd == -1) {
    return std::unexpected(SockError::CREATION);
  }
  return sockfd;
}

template <typename T>
constexpr auto set_sock_opt(int sockfd, int level, int optname, T *value)
    -> std::expected<int, SockError> {
  auto status = setsockopt(sockfd, level, optname, value, sizeof(T));
  if (status == -1) {
    return std::unexpected(SockError::SET_OPTION);
  }
  return sockfd;
}

auto bind_fd(int sockfd, const struct addrinfo *info)
    -> std::expected<int, SockError> {
  auto status = bind(sockfd, info->ai_addr, info->ai_addrlen);
  if (status == -1) {
    return std::unexpected(SockError::BIND);
  }
  return sockfd;
}

// TODO: needs refactor
auto get_valid_address(struct addrinfo *addrinfo, int *can_reuse)
    -> std::expected<int, SockError> {
  for (auto *proto = addrinfo; proto != nullptr; proto = proto->ai_next) {
    auto sockfd = create_socket(proto);
    if (!sockfd.has_value()) {
      freeaddrinfo(addrinfo);
      return std::unexpected(SockError::CREATION);
    }
    auto configured_fd =
        set_sock_opt(sockfd.value(), SOL_SOCKET, SO_REUSEADDR, can_reuse);
    if (!configured_fd.has_value()) {
      freeaddrinfo(addrinfo);
      return std::unexpected(SockError::SET_OPTION);
    };
    auto bound_fd = bind_fd(sockfd.value(), proto);

    if (bound_fd.has_value()) {
      freeaddrinfo(addrinfo);
      return sockfd.value();
    }
    close(sockfd.value());
    break;
  }
  freeaddrinfo(addrinfo);
  return std::unexpected(SockError::NONE_VALID);
}

auto create_tcp_listener(std::string_view addr, std::string_view port,
                         int backlog) -> int {
  struct addrinfo *servinfo = nullptr;

  struct addrinfo hints {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  auto status = getaddrinfo(addr.data(), port.data(), &hints, &servinfo);

  if (status != 0) {
    std::osyncstream(std::cerr)
        << "create_tcp_server: " << gai_strerror(status) << '\n';
  }
  auto sockfd = -1;
  auto *curr = servinfo;
  while (curr != nullptr) {
    sockfd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
    if (sockfd < 0) {
      continue;
    }
    int can_reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &can_reuse, sizeof(int)) <
        0) {
      continue;
    }
    if (bind(sockfd, curr->ai_addr, curr->ai_addrlen) < 0) {
      close(sockfd);
      sockfd = -1;
      continue;
    }
    break;
    curr = curr->ai_next;
  }
  freeaddrinfo(servinfo);

  if (listen(sockfd, backlog) < 0) {
    return -1;
  }
  return sockfd;
}
