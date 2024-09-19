#ifndef EPOLL_WRAPPER_HH
#define EPOLL_WRAPPER_HH
#include <iostream>
#include <optional>
#include <syncstream>
#include <sys/epoll.h>
#include <sys/socket.h>

auto add_to_epoll(int epoll_fd, int client_fd) -> void;

auto remove_from_epoll(int epoll_fd, int client_fd) -> void;

auto create_epoll(int socket_fd) -> std::optional<int>;


#endif // EPOLL_WRAPPER_HH
