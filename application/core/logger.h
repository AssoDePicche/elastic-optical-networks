#pragma once

#include <format>
#include <iostream>
#include <unordered_map>

namespace core {
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
  void Debug(const std::string& format, Args&&... args) {
    log(Logger::Level::Debug, format, args...);
    ;
  }

  template <typename... Args>
  void Error(const std::string& format, Args&&... args) {
    log(Logger::Level::Error, format, args...);
    ;
  }

  template <typename... Args>
  void Info(const std::string& format, Args&&... args) {
    log(Logger::Level::Info, format, args...);
    ;
  }

  template <typename... Args>
  void Warning(const std::string& format, Args&&... args) {
    log(Logger::Level::Warning, format, args...);
    ;
  }

 private:
  std::unordered_map<Level, std::string> _buffer = {
      {Level::Debug, "DEBUG"},
      {Level::Error, "ERROR"},
      {Level::Info, "INFO"},
      {Level::Warning, "WARNING"},
  };

  bool _enableLogging;

  template <typename... Args>
  void log(Level level, const std::string& format, Args&&... args) {
    if (!_enableLogging) {
      return;
    }

    std::clog << std::format(
        "[{}] {}\n", _buffer.at(level),
        std::vformat(format.c_str(), std::make_format_args(args...)));
  }
};
}  // namespace core
