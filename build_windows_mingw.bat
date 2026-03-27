@echo off
REM Build script for Windows with MinGW

REM Create build directory
mkdir build_mingw 2>nul
cd build_mingw

REM Configure with MinGW
cmake .. -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DUSE_OPENMP=OFF ^
    -DUSE_FLOAT=OFF ^
    -DUSE_PER_THREAD_ACCUM=OFF ^
    -DUSE_SIMD=OFF ^
    -DUSE_GPU_COLORMAP=OFF

REM Build
cmake --build . --parallel 4

cd ..
pause