
add_library(
    peer 
    node.cc
    peer.cc
    addr_iter.cc
    socket_wrapper.cc socket_wrapper.hh
    unix_socket.cc unix_socket.hh
)

target_include_directories(peer PUBLIC ../include/peer)

set_property(TARGET peer PROPERTY CXX_STANDARD 23)

