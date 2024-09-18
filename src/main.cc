#include <cstdlib>
#include <iostream>
#include <peer.hh>
#include <unix_socket.hh>

constexpr auto PORT = "3490";
constexpr auto BACKLOG = 10;

auto main(int argc, char **argv) -> int {

  // auto peer = Peer(PORT, BACKLOG);
  // peer.start();
    
  auto socket_path = std::string(getenv("XDG_RUNTIME_DIR")) + "/.test.sock";


  std::cout << socket_path << '\n';
  
  auto sock = UnixSocket(socket_path, BACKLOG);

  sock.start();

  return 0;
}
