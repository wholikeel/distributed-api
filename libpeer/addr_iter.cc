#include <addr_iter.hh>

Iterator::Iterator(pointer ptr) : _ptr(ptr) {}

auto Iterator::operator*() const -> reference { return *_ptr; }
auto Iterator::operator->() const -> pointer { return _ptr; }

auto Iterator::operator++() -> Iterator & {
  _ptr = _ptr->ai_next;
  return *this;
}
auto Iterator::operator++(int) -> Iterator {
  auto tmp = *this;
  ++(*this);
  return tmp;
}


AddrInfoIter::AddrInfoIter(struct addrinfo *addrinfo) : _data(addrinfo) {}
AddrInfoIter::~AddrInfoIter() {
    freeaddrinfo(_data);
}
auto AddrInfoIter::begin() -> Iterator { return {_data}; }
auto AddrInfoIter::end() -> Iterator { return {_data}; }


