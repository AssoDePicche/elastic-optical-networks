#pragma once

#include <format>
#include <iostream>
#include <unordered_map>

class Logger final {
 public:
  enum class Level {
    Debug,
    Error,
    Info,
    Warning,
  };

  Logger(const bool enableLogging) : _enableLogging{enableLogging} {}

  template <typename... Args>
  void log(Level level, const std::string &format, Args &&...args) {
    if (!_enableLogging) {
      return;
    }

    std::clog << std::format(
        "[{}] {}\n", _buffer.at(level),
        std::vformat(format.c_str(), std::make_format_args(args...)));
  }

 private:
  std::unordered_map<Level, std::string> _buffer = {
      {Level::Debug, "DEBUG"},
      {Level::Error, "ERROR"},
      {Level::Info, "INFO"},
      {Level::Warning, "WARNING"},
  };

  bool _enableLogging;
};
