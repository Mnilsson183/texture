# CMakeLists.txt in project_root/

cmake_minimum_required(VERSION 3.10)
project(MyProject VERSION 1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add include directory
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add subdirectories
add_subdirectory(src)

# Enable testing
enable_testing()
