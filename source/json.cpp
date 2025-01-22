#include "json.h"

#include <filesystem>
#include <fstream>

auto read_json(const std::string& filename) -> std::optional<json> {
  if (!std::filesystem::exists(filename)) {
    return std::nullopt;
  }

  std::ifstream stream(filename);

  return json::parse(stream);
}
