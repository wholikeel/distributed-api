
add_library(
    peer 
    node.cc
    peer.cc
    addr_iter.cc
    socket_wrapper.cc socket_wrapper.hh
)

target_include_directories(peer PUBLIC ../include/peer)

set_property(TARGET peer PROPERTY CXX_STANDARD 23)

