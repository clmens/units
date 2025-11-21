cmake_minimum_required(VERSION 3.15)
project(units_console_examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files: main plus the library sources from repo root
add_executable(units_console
    src/main.cpp
    ../../Unit.cpp
    ../../Net.cpp
)

# Make sure the compiler can find headers from repo root
target_include_directories(units_console PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)