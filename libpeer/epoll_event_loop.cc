#include <array>
#include <bit>
#include <epoll_event_loop.hh>
#include <iostream>
#include <sys/epoll.h>

EpollEventLoop::EpollEventLoop(int epoll_fd) : _efd(epoll_fd) {}


auto EpollEventLoop::start(bool *running) -> void {
  constexpr auto event_buf_size = 10;
  auto buffer = std::array<struct epoll_event, event_buf_size>();
  while (*running) {
        auto nfds = epoll_wait(_efd, buffer.data(), event_buf_size, -1);
        for (int idx = 0; idx < nfds; ++idx) {
            if ((buffer.at(idx).events & EPOLLHUP) != 0U) {
                std::cout << "CLOSED\n";
                continue;
            }
            auto *handler =  std::bit_cast<EventHandler *>(buffer.at(idx).data.ptr);
            handler->handler();
        }
  }
}

auto EpollEventLoop::add_event_fd(int client_fd, EventHandler *handler)
    -> void {
  struct epoll_event event {
    .events = EPOLLIN, .data = {.ptr = handler }
  };
  epoll_ctl(_efd, EPOLL_CTL_ADD, client_fd, &event);
}

auto EpollEventLoop::remove_event_fd(int client_fd) -> void {
  epoll_ctl(_efd, EPOLL_CTL_DEL, client_fd, nullptr);
}
