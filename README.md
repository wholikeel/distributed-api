# DAPI

## Running

### Builing

To build the binaries run the following.

```sh
cmake -S . -B out
cmake --build out
```

### Testing

To test, once built run the following.

```sh
ctest --test-dir out
```


## TODO

- [ ] Working Server
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

