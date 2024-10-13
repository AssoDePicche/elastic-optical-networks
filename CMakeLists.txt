cmake_minimum_required(VERSION 3.16)

project(App
  VERSION 0.0.1
  DESCRIPTION "Elastic Optical Network Simulator"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror -DALLOWLOGS)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/source/*.cpp)

foreach(SOURCE ${SOURCES})
  if (NOT ${SOURCE} MATCHES app.cpp)
    get_filename_component(LIBRARY ${SOURCE} NAME_WE)

    add_library(${LIBRARY} SHARED ${SOURCE})
  endif()
endforeach(SOURCE)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/source/app.cpp ${SOURCES})
