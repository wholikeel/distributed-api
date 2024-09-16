#include <peer.hh>

constexpr auto PORT = "3490";
constexpr auto BACKLOG = 10;

auto main() -> int {
  auto peer = Peer(PORT, BACKLOG);
  peer.start();
  return 0;
}
