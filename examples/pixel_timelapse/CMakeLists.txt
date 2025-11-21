cmake_minimum_required(VERSION 3.15)
project(units_pixel_timelapse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Original timelapse example (PPM output)
add_executable(units_pixel_timelapse
    src/main.cpp
    ../../Unit.cpp
    ../../Net.cpp
)

target_include_directories(units_pixel_timelapse PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)

# Core-backed timelapse example (PNG output)
add_executable(units_pixel_timelapse_core
    src/main_png_core.cpp
)

target_include_directories(units_pixel_timelapse_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(units_pixel_timelapse_core PRIVATE units_core)
