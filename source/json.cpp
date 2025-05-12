#include "json.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>

Json::Json(const std::string& filename) {
  if (!std::filesystem::exists(filename)) {
    throw std::runtime_error(std::format("{} not found", filename));
  }

  std::ifstream stream(filename);

  this->json = nlohmann::json::parse(stream);
}

std::vector<std::string> Json::split(const std::string& key,
                                     const char delimiter) const {
  std::vector<std::string> buffer;

  std::stringstream stream(key);

  std::string token;

  while (std::getline(stream, token, delimiter)) {
    buffer.push_back(token);
  }

  return buffer;
}

std::optional<nlohmann::json> Json::unwrap(
    const std::vector<std::string>& buffer, const nlohmann::json& current,
    size_t index) const {
  if (buffer.size() == index) {
    return current;
  }

  const auto& key = buffer[index];

  if (current.is_object() && current.contains(key)) {
    return unwrap(buffer, current[key], index + 1);
  }

  return std::nullopt;
}
