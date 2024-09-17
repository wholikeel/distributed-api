#ifndef PEER_HH
#define PEER_HH
#include <netdb.h>
#include <string_view>
#include <vector>




class Peer {

public:
    Peer(std::string_view port, unsigned int back_log);

    auto start() -> void;
private:
    std::vector<int> _routing_table;
    std::string_view _port;
    unsigned int _back_log;

    auto _start_server();
    auto _client_connect(const std::string &address, const std::string &port);


    constexpr auto _get_hints(int ai_family, int ai_socktype, int ai_flags) -> struct addrinfo;
};


#endif // PEER_HH


