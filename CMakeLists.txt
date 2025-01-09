cmake_minimum_required(VERSION 3.16)

project(App
  VERSION 0.0.1
  DESCRIPTION "Elastic Optical Network Simulator"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "Don't build raylib examples")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build raylib as a static library")

FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 4.5.0
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/raylib
)

FetchContent_MakeAvailable(raylib)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/source/*.cpp)

set(COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Werror -DALLOWLOGS)

foreach(SOURCE ${SOURCES})
  if (NOT ${SOURCE} MATCHES main.cpp)
    get_filename_component(LIBRARY ${SOURCE} NAME_WE)

    add_library(${LIBRARY} SHARED ${SOURCE})

    target_compile_options(${LIBRARY} PRIVATE ${COMPILE_OPTIONS})

    target_link_libraries(${LIBRARY} raylib)
  endif()
endforeach(SOURCE)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/source/main.cpp ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTIONS})

target_link_libraries(${PROJECT_NAME} raylib)
