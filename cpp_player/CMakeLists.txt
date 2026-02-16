cmake_minimum_required(VERSION 3.10)
project(cpp_xm_player VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Source files
set(SOURCES
    src/xm.cpp
    src/integer32.cpp
    src/xmplayer.cpp
)

# Header files
set(HEADERS
    include/mixer.h
    include/xm.h
    include/xmloader.h
    include/xmplayer.h
    include/integer32.h
)

# Create library
add_library(xmplayer STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(xmplayer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Find required libraries
find_package(Threads REQUIRED)
target_link_libraries(xmplayer PRIVATE Threads::Threads)

# Installation
install(TARGETS xmplayer
    EXPORT xmplayerTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${HEADERS} DESTINATION include/xmplayer)

# Export targets
install(EXPORT xmplayerTargets
    FILE xmplayerTargets.cmake
    NAMESPACE xmplayer::
    DESTINATION lib/cmake/xmplayer
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/xmplayerConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/xmplayerConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/xmplayerConfig.cmake"
    INSTALL_DESTINATION lib/cmake/xmplayer
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/xmplayerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/xmplayerConfigVersion.cmake"
    DESTINATION lib/cmake/xmplayer
)

# Build examples if requested
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Enable testing
enable_testing()
