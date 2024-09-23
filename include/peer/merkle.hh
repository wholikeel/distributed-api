#ifndef MERKLE_HH
#define MERKLE_HH
#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#include <string>
#include <variant>

#include <openssl/sha.h>

using HashArr = std::array<unsigned char, SHA_DIGEST_LENGTH>;

template <typename T>
concept IsHashArr = std::same_as<T, HashArr>;

constexpr auto sha1(const std::string &input) {
  HashArr hash{};
  auto *const _ = SHA1(std::bit_cast<const unsigned char *>(input.c_str()),
                       input.size(), hash.data());
  return hash;
}

template <IsHashArr... Hashes>
constexpr auto sha1_combine(const Hashes &...inputs) -> HashArr {
  HashArr combined_input{};
  auto combine = [&](const HashArr &input) {
    for (size_t i = 0; i < SHA_DIGEST_LENGTH; ++i) {
      combined_input[i] ^= input[i];
    }
  };

  (combine(inputs), ...);

  HashArr out{};
  auto *const _ =
      SHA1(combined_input.data(), combined_input.size(), out.data());
  return out;
}

template <typename T> struct Node;
template <typename T> struct Leaf;

template <typename T> using Child = std::variant<Node<T>, Leaf<T>, long>;

template <typename T> struct Node {

  HashArr key;
  std::array<Child<T> *, 4> children;
};

template <typename T> struct Leaf {
  HashArr key;
  T *data;

  ~Leaf() { delete data; }
};

template <typename T> auto hash_data(T data) -> long {}

struct Point {
  long x;
  long y;

  [[nodiscard]] auto as_str() const -> std::string {
    return std::to_string(x) + std::to_string(y);
  }
};



class MerkleQuadTree {

public:
  MerkleQuadTree() {}

  auto insert(Point *data) {
    auto hash = sha1(data->as_str());
    auto new_leaf = Leaf<Point>(hash, data);

    if (!_root.has_value()) {
    }
  }

private:
  std::optional<Node<Point>> _root;
};

#endif // MERKLE_HH
