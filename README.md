# DAPI

>[!WARNING]
>does not work lol

## Building

```sh
cmake -S . -B out
cmake --build out
```

## Testing

```sh
ctest --test-dir out
```


## TODO

- [x] Working Server
- [ ] Working Client
- [ ] Peer-to-Peer network
- [ ] Merkle-tree for distributed event data.
- [ ] Proxy for web clients
- [ ] R-tree (?) for representing received data.
- [ ] CLI client


## References

- [A Suvey of Disributed Graph Algorithms on Massive Scale](https://arxiv.org/abs/2404.06037)
- [Writing a custom iterator in modern C++](https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp)
- [Wikipedia - Merkle tree](https://en.wikipedia.org/wiki/Merkle_tree)
- [Jakob Jenkov - P2P Networks](https://leanpub.com/p2p-networks)
- [Complte Merkle Hash Trees for Large Dynamic Spatial Data](https://ieeexplore.ieee.org/document/9071394)
- [R-trees: A dynamic index structure for spatial searching](https://dl.acm.org/doi/10.1145/971697.602266)

