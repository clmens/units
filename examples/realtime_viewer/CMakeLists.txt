cmake_minimum_required(VERSION 3.15)
project(units_realtime_viewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 QUIET)

if(NOT SDL2_FOUND)
    message(WARNING "SDL2 not found, skipping realtime_viewer build")
    return()
endif()

# Check for OpenGL (for GPU colormap support)
set(HAVE_OPENGL OFF)
if(USE_GPU_COLORMAP)
    find_package(OpenGL QUIET)
    if(OPENGL_FOUND)
        set(HAVE_OPENGL ON)
    endif()
endif()

add_executable(units_realtime_viewer
    src/main.cpp
)

target_link_libraries(units_realtime_viewer PRIVATE
    units_core
    SDL2::SDL2
)

if(HAVE_OPENGL)
    target_link_libraries(units_realtime_viewer PRIVATE OpenGL::GL)
    target_compile_definitions(units_realtime_viewer PRIVATE USE_GPU_COLORMAP)
endif()

target_include_directories(units_realtime_viewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)
