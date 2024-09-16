#ifndef PEER_HH
#define PEER_HH
#include <netdb.h>
#include <string_view>
#include <vector>


enum class SockError {
    CREATION,
    SET_OPTION,
    BIND,
    NONE_VALID
};


class Peer {

public:
    Peer(std::string_view port, unsigned int back_log);

    auto start() -> void;
private:
    std::vector<int> _routing_table;
    std::string_view _port;
    unsigned int _back_log;

    auto _start_listening();

    constexpr auto _get_hints(int ai_family, int ai_socktype, int ai_flags) -> struct addrinfo;
};


#endif // PEER_HH


