cmake_minimum_required(VERSION 3.15)

project(App LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Automatically detect and enable ccache if installed and not explicitly passed
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "ccache enabled: ${CCACHE_PROGRAM}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

enable_testing()

add_subdirectory(core)
add_subdirectory(hash)
add_subdirectory(main)
add_subdirectory(prng)
add_subdirectory(test)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
