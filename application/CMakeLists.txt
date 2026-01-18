cmake_minimum_required(VERSION 3.15)

project(App LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror -flto -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -pg -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -funroll-loops")

find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

enable_testing()

add_subdirectory(core)
add_subdirectory(main)
add_subdirectory(test)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin)
