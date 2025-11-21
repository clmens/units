cmake_minimum_required(VERSION 3.15)
project(units LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options for performance and build configuration
option(USE_OPENMP "Enable OpenMP parallelization" OFF)
option(USE_FLOAT "Use float instead of double for real_t" OFF)
option(USE_PER_THREAD_ACCUM "Use per-thread accumulators in push (requires OpenMP)" OFF)
option(USE_SIMD "Enable SIMD optimizations (experimental)" OFF)

# Conditionally enable GPU colormap (ON by default if OpenGL is available)
find_package(OpenGL QUIET)
if(OPENGL_FOUND)
    option(USE_GPU_COLORMAP "Enable GPU-accelerated colormap in realtime_viewer (requires OpenGL)" ON)
else()
    option(USE_GPU_COLORMAP "Enable GPU-accelerated colormap in realtime_viewer (requires OpenGL)" OFF)
endif()

# OpenMP support
if(USE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        message(STATUS "OpenMP enabled")
    else()
        message(WARNING "USE_OPENMP=ON but OpenMP not found")
    endif()
endif()

# Define real_t based on USE_FLOAT option
if(USE_FLOAT)
    add_compile_definitions(USE_FLOAT)
    message(STATUS "Using float for real_t")
else()
    message(STATUS "Using double for real_t")
endif()

# Define USE_PER_THREAD_ACCUM if enabled
if(USE_PER_THREAD_ACCUM)
    if(NOT USE_OPENMP)
        message(WARNING "USE_PER_THREAD_ACCUM requires USE_OPENMP=ON to have effect")
    endif()
    add_compile_definitions(USE_PER_THREAD_ACCUM)
    message(STATUS "Per-thread accumulators enabled")
endif()

# SIMD optimizations (experimental)
if(USE_SIMD)
    message(STATUS "SIMD optimizations enabled (experimental)")
    add_compile_definitions(USE_SIMD)
    if(NOT MSVC)
        add_compile_options(-msse4.2)
    endif()
endif()

# Static library for UnitsCore
add_library(units_core STATIC
    src/units_core.cpp
    src/units_core.h
)

target_include_directories(units_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(USE_OPENMP AND OpenMP_CXX_FOUND)
    target_link_libraries(units_core PUBLIC OpenMP::OpenMP_CXX)
endif()

# Add subdirectories
# Note: console_app and pixel_timelapse use legacy Unit/Net classes with Cinder dependency
# They are excluded from the build unless Cinder is available
add_subdirectory(bench)

# Optional: add realtime_viewer if SDL2 is available
find_package(SDL2 QUIET)
if(SDL2_FOUND)
    add_subdirectory(examples/realtime_viewer)
    message(STATUS "SDL2 found - realtime_viewer will be built")
else()
    message(STATUS "SDL2 not found - realtime_viewer will not be built")
endif()
