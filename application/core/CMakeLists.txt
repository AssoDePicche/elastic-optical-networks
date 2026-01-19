add_library(core STATIC
  configuration.cpp
  csv.cpp
  file_system.cpp
  graph.cpp
  json.cpp
  kernel.cpp
  prng.cpp
  request.cpp
  route.cpp
  spectrum.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(core PUBLIC nlohmann_json::nlohmann_json)
