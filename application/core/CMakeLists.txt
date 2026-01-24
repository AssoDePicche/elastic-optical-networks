add_library(core STATIC
  application.cpp
  configuration.cpp
  file_system.cpp
  json.cpp
  kernel.cpp
  request.cpp
  spectrum.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(core PUBLIC graph hash prng nlohmann_json::nlohmann_json)
