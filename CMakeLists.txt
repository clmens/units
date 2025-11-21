cmake_minimum_required(VERSION 3.15)
project(units VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Performance and precision options
option(USE_OPENMP "Enable OpenMP parallelization" OFF)
option(USE_FLOAT "Use float precision instead of double" OFF)
option(USE_PER_THREAD_ACCUM "Use per-thread accumulator to avoid atomics (requires OpenMP)" OFF)
option(USE_SIMD "Enable SIMD optimizations (-march=native)" OFF)
option(USE_GPU_COLORMAP "Enable GPU-accelerated color mapping in viewer (requires OpenGL)" ON)

# Build options
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_BENCH "Build benchmark tools" ON)

# Core library
add_library(units_core STATIC
    src/units_core.cpp
    src/units_core.h
)

target_include_directories(units_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Apply compile definitions
if(USE_FLOAT)
    target_compile_definitions(units_core PUBLIC USE_FLOAT)
endif()

if(USE_PER_THREAD_ACCUM)
    target_compile_definitions(units_core PUBLIC USE_PER_THREAD_ACCUM)
endif()

if(USE_GPU_COLORMAP)
    target_compile_definitions(units_core PUBLIC USE_GPU_COLORMAP)
endif()

# OpenMP support
if(USE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(units_core PUBLIC OpenMP::OpenMP_CXX)
    else()
        message(WARNING "OpenMP requested but not found")
    endif()
endif()

# Compiler optimizations
if(USE_SIMD)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(units_core PUBLIC -march=native -O3)
    elseif(MSVC)
        target_compile_options(units_core PUBLIC /O2 /arch:AVX2)
    endif()
else()
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(units_core PUBLIC -O3)
    elseif(MSVC)
        target_compile_options(units_core PUBLIC /O2)
    endif()
endif()

# Subdirectories
if(BUILD_BENCH)
    add_subdirectory(bench)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples/console_app)
    add_subdirectory(examples/pixel_timelapse)
    add_subdirectory(examples/realtime_viewer)
endif()
