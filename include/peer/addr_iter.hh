#ifndef ADDR_ITER_HH
#define ADDR_ITER_HH
#include <cstddef>
#include <iterator>

#include <netdb.h>

using iterator_category = std::forward_iterator_tag;
using difference_type = std::ptrdiff_t;
using value_type = struct addrinfo;
using pointer = struct addrinfo *;
using reference = struct addrinfo &;


class Iterator {
public:
    Iterator(pointer ptr);

    auto operator*() const -> reference;
    auto operator->() const -> pointer;

    auto operator++() -> Iterator &;
    auto operator++(int) -> Iterator;

    pointer _ptr;
};

class AddrInfoIter {
public:
  AddrInfoIter(struct addrinfo *addrinfo);
  ~AddrInfoIter();
  auto begin() -> Iterator;
  auto end() -> Iterator;

private:
  struct addrinfo *_data;
};

inline auto operator==(const Iterator &lhs, const Iterator &rhs) -> bool {
  return lhs._ptr->ai_canonname == rhs._ptr->ai_canonname;
}
inline auto operator!=(const Iterator &lhs, const Iterator &rhs) -> bool {
  return lhs._ptr->ai_canonname != rhs._ptr->ai_canonname;
}

#endif // ADDR_ITER_HH
