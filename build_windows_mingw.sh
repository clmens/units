#!/bin/bash
# Build script for Windows with MinGW

# Create build directory
mkdir -p build_mingw
cd build_mingw

# Configure with MinGW
cmake .. -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_OPENMP=OFF \
    -DUSE_FLOAT=OFF \
    -DUSE_PER_THREAD_ACCUM=OFF \
    -DUSE_SIMD=OFF \
    -DUSE_GPU_COLORMAP=OFF

# Build
cmake --build . --parallel 4

cd ..
echo "Build completed. Executables are in build_mingw/"