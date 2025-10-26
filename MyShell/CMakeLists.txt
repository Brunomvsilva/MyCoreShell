cmake_minimum_required(VERSION 3.13)

project(shell-starter-cpp LANGUAGES CXX)

# Use C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect C++ source files
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

# Executable
add_executable(shell ${SOURCE_FILES})

# Include src directory for headers
target_include_directories(shell PRIVATE src)

# Link libs (readline)
target_link_libraries(shell PRIVATE readline)

# Enable useful warnings
target_compile_options(shell PRIVATE -Wall -Wextra -Wpedantic)
