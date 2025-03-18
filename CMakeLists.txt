cmake_minimum_required(VERSION 3.21)
project(SpoofDeviceModule)

add_library(
    spoof
    SHARED
    zygisk/spoof.cpp
)

target_include_directories(
    spoof
    PRIVATE
    ${CMAKE_SOURCE_DIR}/zygisk
)

target_link_libraries(
    spoof
    log
)

# Remove this line to let NDK set the output name based on ANDROID_ABI
# set_target_properties(spoof PROPERTIES OUTPUT_NAME "arm64-v8a")
