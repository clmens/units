cmake_minimum_required(VERSION 3.15)
project(units_pixel_timelapse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(units_pixel_timelapse
    src/main.cpp
    ../../Unit.cpp
    ../../Net.cpp
)

target_include_directories(units_pixel_timelapse PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)