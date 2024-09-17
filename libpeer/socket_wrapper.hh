#ifndef SOCKET_WRAPPER_HH
#define SOCKET_WRAPPER_HH

#include <arpa/inet.h>
#include <csignal>
#include <expected>
#include <iostream>
#include <netdb.h>
#include <optional>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <addr_iter.hh>
#include <array>
#include <string>
#include <unistd.h>
#include <utility>



enum class SockError {
    CREATION,
    SET_OPTION,
    BIND,
    NONE_VALID
};

auto send(int file_descriptor, std::string_view message,
                    int flags = 0) -> ssize_t;

auto get_in_addr(struct sockaddr_storage *storage)
    -> std::expected<std::string, int>;

auto accept_conn(int sockfd)
    -> std::optional<std::pair<int, struct sockaddr_storage>>;

auto get_addr_info(std::string_view hostname, std::string_view port, struct addrinfo hints)
    -> std::expected<struct addrinfo *, int>;

auto create_socket(struct addrinfo *sockaddr)
    -> std::expected<int, SockError>;

template <typename T>
constexpr auto set_sock_opt(int sockfd, int level, int optname, T *value)
    -> std::expected<int, SockError>;

auto bind_fd(int sockfd, const struct addrinfo *info)
    -> std::expected<int, SockError>;

// TODO: needs api change
auto get_valid_address(struct addrinfo *addrinfo, int *can_reuse)
    -> std::expected<int, SockError>;

#endif // SOCKET_WRAPPER_HH
