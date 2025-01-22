#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

using json = nlohmann::json;

[[nodiscard]] auto read_json(const std::string&) -> std::optional<json>;
