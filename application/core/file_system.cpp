#include "file_system.h"

#include <filesystem>
#include <iostream>

namespace core {
bool FileSystem::CreateDirectory(const std::string& pathname) {
  std::filesystem::path path = pathname;

  std::error_code errorCode;

  if (std::filesystem::create_directories(path, errorCode)) {
    return true;
  }

  std::cerr << "Failed to create directory: " << errorCode.message()
            << std::endl;

  return false;
}
}  // namespace core
