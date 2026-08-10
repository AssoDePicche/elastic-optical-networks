add_library(core STATIC
  agent.cpp
  cli.cpp
  configuration.cpp
  cost.cpp
  dijkstra.cpp
  file_system.cpp
  flexgrid.cpp
  graph.cpp
  kernel.cpp
  request.cpp
  router.cpp
  spectrum.cpp
  yen.cpp
)

find_package(Boost REQUIRED)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(core PUBLIC hash prng Boost::boost nlohmann_json::nlohmann_json)
