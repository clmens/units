cmake_minimum_required(VERSION 3.15)
project(units_realtime_viewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)

add_executable(realtime_viewer
    src/main.cpp
)

target_link_libraries(realtime_viewer PRIVATE units_core SDL2::SDL2)

# Set optimization flags for Release builds
# Using -O3 for consistency with benchmark tool
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if(MSVC)
        target_compile_options(realtime_viewer PRIVATE /O2)
    else()
        target_compile_options(realtime_viewer PRIVATE -O3)
    endif()
endif()
