cmake_minimum_required(VERSION 3.10)

# Simple example
add_executable(simple_example simple_example.cpp)
target_link_libraries(simple_example PRIVATE xmplayer)

# Install examples
install(TARGETS simple_example DESTINATION bin/examples)
