cmake_minimum_required(VERSION 3.21)
project(SpoofModule)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the spoof library
add_library(spoof SHARED
    spoof_module.cpp
)

# Find log library
find_library(log-lib log)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/zygisk)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/AndroidJNIHelper)  

# Link libraries
target_link_libraries(spoof PRIVATE ${log-lib})
