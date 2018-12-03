cmake_minimum_required(VERSION 3.10)

project(iter LANGUAGES CXX VERSION 0.1)

# Tests
find_package(GTest REQUIRED)
add_subdirectory(
  /usr/src/googletest
  ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
add_subdirectory(test)
