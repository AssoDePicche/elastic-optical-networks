#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

[[nodiscard]] std::optional<nlohmann::json> read_json(const std::string&);
