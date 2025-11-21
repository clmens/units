cmake_minimum_required(VERSION 3.15)

# Benchmark executable
add_executable(bench_units benchmark.cpp)

target_link_libraries(bench_units PRIVATE units_core)

# Set optimization flags for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if(MSVC)
        target_compile_options(bench_units PRIVATE /O2)
    else()
        target_compile_options(bench_units PRIVATE -O3)
    endif()
endif()
