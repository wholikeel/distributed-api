#include "epoll_wrapper.hh"


auto add_to_epoll(int epoll_fd, int client_fd) -> void {
  struct epoll_event epoll_event {};
  epoll_event.events = EPOLLIN;
  epoll_event.data.fd = client_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &epoll_event) == -1) {
    std::osyncstream(std::cerr) << "epoll add error\n";
  }
};

auto remove_from_epoll(int epoll_fd, int client_fd) -> void {
  struct epoll_event epoll_event {};
  epoll_event.events = EPOLLIN;
  epoll_event.data.fd = client_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, &epoll_event) == -1) {
    std::osyncstream(std::cerr) << "epoll remove error\n";
  }
}

auto create_epoll(int socket_fd) -> std::optional<int> {
  auto efd = epoll_create1(EPOLL_CLOEXEC);
  if (efd == -1) {
    return std::nullopt;
  }
  add_to_epoll(efd, socket_fd);
  return efd;
}





