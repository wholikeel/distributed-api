#ifndef TCP_HANDLER_HH
#define TCP_HANDLER_HH

#include "event_handler.hh"
#include "event_loop.hh"
#include <memory>


class EchoHandler : public EventHandler {
public:
    EchoHandler(std::shared_ptr<EventLoop> event_loop, int client_fd);
    auto handler() -> void override;
private:
    int _client_fd;
    std::shared_ptr<EventLoop> _ev;
};



class TcpConnectionHandler : public EventHandler {
public:
    TcpConnectionHandler(std::shared_ptr<EventLoop> event_loop, int server_fd);
    auto handler() -> void override;

private:
    int _server_fd;
    std::shared_ptr<EventLoop> _ev;
};


#endif // TCP_HANDLER_HH
