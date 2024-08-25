cmake_minimum_required(VERSION 3.10)

project(iter LANGUAGES CXX VERSION 0.1)

add_library(iter INTERFACE)
target_include_directories(iter INTERFACE ${CMAKE_SOURCE_DIR}/src)

# Tests
enable_testing()
add_subdirectory(test)
