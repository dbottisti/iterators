cmake_minimum_required(VERSION 3.10)

project(iter LANGUAGES CXX VERSION 0.1)

add_library(iter INTERFACE)
target_include_directories(iter INTERFACE ${CMAKE_SOURCE_DIR}/src)

# Tests
find_package(GTest REQUIRED)
add_subdirectory(
  /usr/src/googletest
  ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
add_subdirectory(test)
