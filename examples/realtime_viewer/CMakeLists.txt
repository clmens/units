cmake_minimum_required(VERSION 3.15)
project(units_realtime_viewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 QUIET)

if(NOT SDL2_FOUND)
    message(STATUS "SDL2 not found, skipping realtime_viewer")
    return()
endif()

# Find OpenGL if GPU colormap is enabled
if(USE_GPU_COLORMAP)
    find_package(OpenGL QUIET)
    if(NOT OPENGL_FOUND)
        message(STATUS "OpenGL not found, disabling GPU color mapping")
        set(USE_GPU_COLORMAP OFF)
    endif()
endif()

add_executable(units_realtime_viewer src/main.cpp)

target_link_libraries(units_realtime_viewer PRIVATE units_core SDL2::SDL2)

if(USE_GPU_COLORMAP AND OPENGL_FOUND)
    target_link_libraries(units_realtime_viewer PRIVATE OpenGL::GL)
endif()

target_include_directories(units_realtime_viewer PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src
)
