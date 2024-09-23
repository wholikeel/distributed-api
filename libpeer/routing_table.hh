#ifndef ROUTING_TABLE_HH
#define ROUTING_TABLE_HH
#include <atomic>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <vector>


struct ContactInfo {
    int id;
    std::string host;
    std::string port;
};

class ChordRoutingTable {
public:
  ChordRoutingTable();

  auto get_largest_successor() -> int {
    std::lock_guard<std::mutex> lock(_peer_fds_mutex);
    return _peer_fds.empty() ? _id : *_peer_fds.end();
  }

  auto get_closest_to(int target_id) -> ContactInfo {
    std::lock_guard<std::mutex> lock(_peer_fds_mutex);
    if (target_id - _id >= 0) {
        return {_id, "", ""};
    }
    size_t idx = std::size(_peer_fds) - 1;
    for (; idx >= 0; idx--) {
        if (target_id <= idx + target_id) {
            break;
        }
    }
    // auto peer = getpeername();
  }

private:
  std::vector<int> _peer_fds;
  std::mutex _peer_fds_mutex;

  int _id;
};

#endif // ROUTING_TABLE_HH
