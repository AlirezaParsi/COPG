cmake_minimum_required(VERSION 3.10)
project(SpoofModule)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(zygisk include/nlohmann)

# Source files
add_library(spoof SHARED spoof_module.cpp)


# Set output name
set_target_properties(spoof PROPERTIES OUTPUT_NAME "spoof")
