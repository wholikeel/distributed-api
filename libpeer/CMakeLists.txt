
add_library(
    peer 
    node.cc
)

target_include_directories(peer PUBLIC ../include/peer)

set_property(TARGET peer PROPERTY CXX_STANDARD 23)

