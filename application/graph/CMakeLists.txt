add_library(graph STATIC
  bfs.cpp
  cost.cpp
  csv.cpp
  dijkstra.cpp
  dfs.cpp
  graph.cpp
  ksp.cpp
  route.cpp
  router.cpp
)

find_package(Boost REQUIRED)

target_include_directories(graph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(graph PRIVATE Boost::boost hash)
