#ifndef EPOLL_EVENT_LOOP_HH
#define EPOLL_EVENT_LOOP_HH
// #include "event_handler.hh"
// #include "event_loop.hh"
#include <event_loop.hh>
#include <event_handler.hh>
#include <functional>

class EpollEventLoop : public EventLoop {
public:
  EpollEventLoop(int epoll_fd);
  ~EpollEventLoop() override = default;
  auto start(bool *running) -> void override;
  auto add_event_fd(int client_fd, EventHandler *handler) -> void override;
  auto remove_event_fd(int client_fd) -> void override;

private:
  int _efd;
};

#endif // EPOLL_EVENT_LOOP_HH
