cmake_minimum_required(VERSION 3.15)
project(units LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(USE_OPENMP "Enable OpenMP parallelization" OFF)
option(USE_FLOAT "Use float instead of double for UnitsCore" OFF)
option(USE_PER_THREAD_ACCUM "Enable per-thread accumulator for push (requires OpenMP)" OFF)
option(USE_GPU_COLORMAP "Enable GPU color mapping in realtime viewer (requires OpenGL)" ON)
option(USE_SIMD "Enable SIMD optimizations (-march=native)" OFF)

# Optimization flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(USE_SIMD)
        add_compile_options(-march=native -O3)
    else()
        add_compile_options(-O3)
    endif()
endif()

# Compile definitions
if(USE_FLOAT)
    add_compile_definitions(USE_FLOAT)
endif()

if(USE_PER_THREAD_ACCUM)
    add_compile_definitions(USE_PER_THREAD_ACCUM)
endif()

if(USE_GPU_COLORMAP)
    add_compile_definitions(USE_GPU_COLORMAP)
endif()

# OpenMP support
if(USE_OPENMP)
    find_package(OpenMP)
    if(NOT OpenMP_CXX_FOUND)
        message(WARNING "OpenMP requested but not found")
    endif()
endif()

# Build UnitsCore as a static library
add_library(units_core STATIC
    src/units_core.cpp
    src/units_core.h
)

target_include_directories(units_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(USE_OPENMP AND OpenMP_CXX_FOUND)
    target_link_libraries(units_core PUBLIC OpenMP::OpenMP_CXX)
endif()

# Add subdirectories
add_subdirectory(bench)

# Legacy examples require cinder framework - skip if not available
# add_subdirectory(examples/console_app)
# add_subdirectory(examples/pixel_timelapse)

# Check if realtime_viewer exists and add it
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/realtime_viewer/CMakeLists.txt)
    add_subdirectory(examples/realtime_viewer)
endif()
