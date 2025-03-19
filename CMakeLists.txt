cmake_minimum_required(VERSION 3.21)
project(SpoofModule)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(spoof SHARED spoof_module.cpp)

find_library(log-lib log)

include_directories(${CMAKE_SOURCE_DIR}/zygisk)
include_directories(${CMAKE_SOURCE_DIR}/include)

target_link_libraries(spoof ${log-lib})
