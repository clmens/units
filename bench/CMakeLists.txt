cmake_minimum_required(VERSION 3.15)

add_executable(bench_units
    benchmark.cpp
)

target_link_libraries(bench_units PRIVATE units_core)

target_include_directories(bench_units PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
