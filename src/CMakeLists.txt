cmake_minimum_required(VERSION 3.21)

# Bundle libc++ statically: Zygisk injects this .so into app processes that have
# NO libc++_shared.so in their linker namespace (it's not in /system), so a
# dynamic libc++ dependency makes dlopen fail and the module never loads.
# Must be set before project() so the NDK toolchain picks it up.
set(ANDROID_STL c++_static)

project(SpoofModule)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Os -flto -ffunction-sections -fdata-sections)
add_link_options(-flto -Wl,--gc-sections)

if(NOT DEBUG)
    add_compile_options(-g0 -DNDEBUG)
    add_link_options(-s)
endif()

# Zygisk companion (.so).
add_library(spoof SHARED
            spoof_module.cpp
            atexit.cpp)

find_library(log-lib log)

# zygisk.hpp + json.hpp both live in src/include
target_include_directories(spoof PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(spoof ${log-lib})

set_target_properties(spoof PROPERTIES
    LINK_FLAGS "-Wl,--exclude-libs,ALL"
)
