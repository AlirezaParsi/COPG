cmake_minimum_required(VERSION 3.21)
project(SpoofModule)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(zygisk include/nlohmann dobby)

# Source files
add_library(spoof SHARED spoof_module.cpp)

# Find and link Dobby
add_library(dobby STATIC IMPORTED)
set_target_properties(dobby PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/dobby/libdobby.a"
)

# Link libraries
target_link_libraries(spoof dobby log)

# Set output name
set_target_properties(spoof PROPERTIES OUTPUT_NAME "spoof")
