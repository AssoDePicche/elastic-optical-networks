#include "file_system.h"

#include <filesystem>
#include <iostream>

namespace core::file_system {
bool create_directory(const std::string& pathname) {
  std::filesystem::path path = pathname;

  std::error_code errorCode;

  if (std::filesystem::exists(path)) {
    std::cerr << "Failed to create directory" << std::endl;

    return false;
  }

  if (std::filesystem::create_directories(path, errorCode)) {
    return true;
  }

  std::cerr << "Failed to create directory: " << errorCode.message()
            << std::endl;

  return false;
}

bool exists(const std::string& pathname) {
  return std::filesystem::exists(pathname);
}
}  // namespace core::file_system
