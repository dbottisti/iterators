add_executable(vector_test vector_test.cpp)
target_compile_features(vector_test PUBLIC cxx_std_17)
target_link_libraries(vector_test iter gmock gmock_main)
gtest_discover_tests(vector_test)
