cmake_minimum_required(VERSION 3.10)

# Simple example
add_executable(simple_example simple_example.cpp)
target_link_libraries(simple_example PRIVATE xmplayer)

# Test loader example
add_executable(test_loader test_loader.cpp)
target_link_libraries(test_loader PRIVATE xmplayer)

# Install examples
install(TARGETS simple_example test_loader DESTINATION bin/examples)
