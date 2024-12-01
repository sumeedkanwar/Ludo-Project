cmake_minimum_required(VERSION 3.10)

# Set the project name
project(LudoGame)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(LudoGame main.cpp)

# Find SFML package
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Link SFML libraries
target_link_libraries(LudoGame sfml-graphics sfml-window sfml-system)