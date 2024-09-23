#ifndef PEER_COMMUNICATION_PROTOCOL_HH
#define PEER_COMMUNICATION_PROTOCOL_HH
#include <algorithm>
#include <cassert>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <variant>
#include <vector>

struct RequestID {};
struct ResponseID {
  int id;
};

struct RequestSuccessor {
    int id;
};

struct ResponseSuccessor {
    int id;
    std::string host;
    std::string port;
};

struct RequestClosestPreceding {
    int id;
};

struct ResponseClosestPreceding {
    int id;
    std::string host;
    std::string port;
};

struct RequestLargest {
};

struct ResponseLargest {
    int id;
    std::string host;
    std::string port;
};

enum class MessageType {
  RequestID,
  ResponseID,
};

using MessageData = std::variant<RequestID, ResponseID, std::monostate>;

struct PCPData {
  MessageType type;
  MessageData data;
};

auto pcp_serialize(PCPData data) -> std::optional<std::string> {
  std::string message = std::to_string((int)data.type);

  switch (data.type) {
  case MessageType::RequestID:
    break;
  case MessageType::ResponseID: {
    auto req_data = std::get<ResponseID>(data.data);
    message += ',' + std::to_string(req_data.id);
  } break;
  default:
    return std::nullopt;
  }
  return message + '\0';
}

template <typename T>
concept Numeric = std::integral<T>;

template <Numeric T>
auto _parse_num(const std::string &input, std::size_t size) -> T {
  T acc = 0;
  for (std::size_t idx = 0; idx < size; idx++) {
    acc = (acc * 10) + (((T)input[idx]) - 48);
  }
  return acc;
};

auto _str_split(std::string::const_iterator begin,
                std::string::const_iterator end, char delim) {
  std::vector<std::string> ret_val = {""};
  size_t size = 0;
  for (auto curr = begin; curr != end; curr++) {
    if (*curr == delim) {
      size++;
      ret_val.emplace_back("");
    } else {
      ret_val[size].push_back(*curr);
    }
  }
  return ret_val;
}

auto pcp_deserialize(const std::string &value) -> std::optional<PCPData> {
  if (value.empty()) {
    return std::nullopt;
  }
  auto count = std::count_if(value.cbegin(), value.cend(),
                             [](unsigned char elem) { return isdigit(elem); });

  auto type = (MessageType)_parse_num<int>(value, count);
  MessageData data = std::monostate();

  auto parts = _str_split(value.cbegin(), value.cbegin() + count, ',');

  switch (type) {
  case MessageType::RequestID:
    data = RequestID{};
    break;
  case MessageType::ResponseID: {
    if (parts.empty()) {
      return std::nullopt;
    }
    data = ResponseID(_parse_num<int>(parts[0], parts.size()));
  } break;
  default:
    return std::nullopt;
  }
  return PCPData(type, data);
}

#endif
