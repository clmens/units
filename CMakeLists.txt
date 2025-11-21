cmake_minimum_required(VERSION 3.15)
project(units LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options for performance tuning
option(USE_FLOAT "Use float precision instead of double" OFF)
option(USE_OPENMP "Enable OpenMP parallelism" OFF)

# Configure float/double precision
if(USE_FLOAT)
    add_compile_definitions(UNITS_USE_FLOAT)
endif()

# Enable OpenMP if requested
if(USE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        message(STATUS "OpenMP enabled")
    else()
        message(WARNING "OpenMP requested but not found")
    endif()
endif()

# UnitsCore library target
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

# Examples (optional subdirectories)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
    add_subdirectory(examples/console_app EXCLUDE_FROM_ALL)
    add_subdirectory(examples/pixel_timelapse EXCLUDE_FROM_ALL)
endif()
