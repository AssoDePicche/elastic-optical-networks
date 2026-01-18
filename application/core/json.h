#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

class Json final {
 public:
  Json(const std::string&);

  template <typename T>
  std::optional<T> Get(const std::string& key) const {
    const auto j = unwrap(split(key), json);

    if (j.has_value()) {
      return j.value().get<T>();
    }

    return std::nullopt;
  }

 private:
  nlohmann::json json;

  std::vector<std::string> split(const std::string&, const char = '.') const;

  std::optional<nlohmann::json> unwrap(const std::vector<std::string>&,
                                       const nlohmann::json&, size_t = 0) const;
};
