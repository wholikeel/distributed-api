#ifndef EVENT_LOOP_HH
#define EVENT_LOOP_HH
#include <sys/epoll.h>
#include "event_handler.hh"


class EventLoop {
public:
    virtual ~EventLoop() = 0;

    virtual auto start(bool *running) -> void = 0;

    virtual auto add_event_fd(int client_fd, EventHandler *handler) -> void = 0;

    virtual auto remove_event_fd(int client_fd) -> void = 0;
};

#endif // EVENT_LOOP_HH
