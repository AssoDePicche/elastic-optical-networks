cmake_minimum_required(VERSION 3.16)

project(App CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Werror -DALLOWLOGS -g)

set(DEPENDENCIES nlohmann_json gtest gtest_main)

set(INCLUDE ${CMAKE_SOURCE_DIR}/include)

set(SOURCE ${CMAKE_SOURCE_DIR}/source)

set(TEST ${CMAKE_SOURCE_DIR}/test)

set(TESTS ${PROJECT_NAME}Test)

add_subdirectory(vendor/json)

add_subdirectory(vendor/googletest)

file(GLOB SOURCES ${SOURCE}/*.cpp)

list(REMOVE_ITEM SOURCES ${SOURCE}/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCE}/main.cpp ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE})

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTIONS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${DEPENDENCIES})

enable_testing()

file(GLOB TEST_SOURCES ${TEST}/*.cpp)

add_executable(${TESTS} ${TEST_SOURCES} ${SOURCES})

target_include_directories(${TESTS} PRIVATE ${INCLUDE} ${SOURCE})

target_compile_options(${TESTS} PRIVATE ${COMPILE_OPTIONS})

target_link_libraries(${TESTS} PRIVATE ${DEPENDENCIES})

add_test(NAME ${TESTS} COMMAND ${TESTS} --gtest_color=1)
