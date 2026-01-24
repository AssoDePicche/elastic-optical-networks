add_library(prng STATIC
  discrete.cpp
  distribution.cpp
  exponential.cpp
  normal.cpp
  poisson.cpp
  prng.cpp
  uniform.cpp
)

target_include_directories(prng PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
