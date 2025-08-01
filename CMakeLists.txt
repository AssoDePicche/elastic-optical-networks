cmake_minimum_required(VERSION 3.16)

project(App CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Werror -pg)

set(APP_DIR ${CMAKE_SOURCE_DIR}/application)

set(CORE_INCLUDE ${APP_DIR}/core/include)

set(CORE_SOURCE ${APP_DIR}/core/source
)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)

set(DEPENDENCIES nlohmann_json gtest gtest_main)

add_subdirectory(vendor/json)

add_subdirectory(vendor/googletest)

file(GLOB CORE_SOURCES ${CORE_SOURCE}/*.cpp)

add_executable(${PROJECT_NAME} ${APP_DIR}/main.cpp ${CORE_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CORE_INCLUDE})

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTIONS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${DEPENDENCIES})

target_link_options(${PROJECT_NAME} PRIVATE -pg)

enable_testing()

file(GLOB TEST_SOURCES ${TEST_DIR}/*.cpp)

add_executable(${PROJECT_NAME}Test ${TEST_SOURCES} ${CORE_SOURCES})

target_include_directories(${PROJECT_NAME}Test PRIVATE ${CORE_INCLUDE} ${CORE_SOURCE})

target_compile_options(${PROJECT_NAME}Test PRIVATE ${COMPILE_OPTIONS})

target_link_libraries(${PROJECT_NAME}Test PRIVATE ${DEPENDENCIES})

target_link_options(${PROJECT_NAME}Test PRIVATE -pg)

add_test(NAME ${PROJECT_NAME}Test COMMAND ${PROJECT_NAME}Test --gtest_color=1)
