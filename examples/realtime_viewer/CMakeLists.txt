cmake_minimum_required(VERSION 3.15)
project(units_realtime_viewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option for GPU colormap (requires OpenGL)
option(USE_GPU_COLORMAP "Enable GPU-accelerated colormap rendering (requires OpenGL)" ON)

# Find SDL2
find_package(SDL2 REQUIRED)

# Optionally find OpenGL if GPU colormap is enabled
if(USE_GPU_COLORMAP)
    find_package(OpenGL QUIET)
    if(OPENGL_FOUND)
        message(STATUS "OpenGL found - GPU colormap will be enabled")
        add_compile_definitions(USE_GPU_COLORMAP)
    else()
        message(STATUS "OpenGL not found - GPU colormap disabled, using CPU fallback")
    endif()
endif()

add_executable(realtime_viewer
    src/main.cpp
)

target_link_libraries(realtime_viewer PRIVATE units_core SDL2::SDL2)

# Link OpenGL if GPU colormap is enabled
if(USE_GPU_COLORMAP AND OPENGL_FOUND)
    target_link_libraries(realtime_viewer PRIVATE OpenGL::GL)
endif()

# Set optimization flags for Release builds
# Using -O3 for consistency with benchmark tool
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if(MSVC)
        target_compile_options(realtime_viewer PRIVATE /O2)
    else()
        target_compile_options(realtime_viewer PRIVATE -O3)
    endif()
endif()
