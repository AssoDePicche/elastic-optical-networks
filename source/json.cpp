#include "json.h"

#include <filesystem>
#include <fstream>

std::optional<nlohmann::json> read_json(const std::string& filename) {
  if (!std::filesystem::exists(filename)) {
    return std::nullopt;
  }

  std::ifstream stream(filename);

  return nlohmann::json::parse(stream);
}
