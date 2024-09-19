#ifndef PEER_HH
#define PEER_HH
#include <atomic>
#include <netdb.h>
#include <string_view>
#include <vector>
#include <string>
#include <thread>
#include <mutex>




class Peer {

public:
    Peer(std::string port, int back_log);
    ~Peer();

    auto start() -> void;
    auto stop() -> void;

    auto add_entry(int data) -> void;
private:
    int _id = 0;
    std::string _port;
    const int _back_log;
    std::vector<int> _data_entries;

    // std::string _data_hash;

    std::vector<std::string> _routing_table;
    std::mutex _routing_table_mutex;

    std::atomic<bool> _is_listening = false;
    std::thread _server_thread;


    auto _start_server() -> int;
    auto _client_connect(const std::string &address, const std::string &port);
    auto _update_hash();


    auto _add_peer(const std::string &peer) -> void;
    // auto _find_next(const std::string &peer) -> void;


    constexpr auto _get_hints(int ai_family, int ai_socktype, int ai_flags) -> struct addrinfo;
};


#endif // PEER_HH


