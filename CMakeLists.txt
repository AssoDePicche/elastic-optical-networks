cmake_minimum_required(VERSION 3.16)

project(App
  VERSION 0.0.1
  DESCRIPTION "Elastic Optical Network Simulator"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(source)
