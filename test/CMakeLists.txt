find_package(Catch2 3 REQUIRED)

add_executable(iterators iterators.cpp)
target_compile_features(iterators PUBLIC cxx_std_17)
target_link_libraries(iterators PRIVATE iter Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(iterators)