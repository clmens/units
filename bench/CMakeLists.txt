cmake_minimum_required(VERSION 3.15)
project(bench_units LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(bench_units benchmark.cpp)

# Link to units_core static library
target_link_libraries(bench_units PRIVATE units_core)

# Include directory for units_core headers
target_include_directories(bench_units PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

# OpenMP support if enabled
if(USE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(bench_units PRIVATE OpenMP::OpenMP_CXX)
    endif()
endif()
