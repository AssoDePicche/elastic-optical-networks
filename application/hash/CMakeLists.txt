add_library(hash STATIC
  cantor.cpp
)

target_include_directories(hash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
