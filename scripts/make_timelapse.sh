#!/usr/bin/env bash
set -e

EX=examples/pixel_timelapse
mkdir -p $EX/build
pushd $EX/build
cmake ..
cmake --build .
popd

# run example
$EX/build/units_pixel_timelapse --width 100 --height 100 --steps 200 --seed 42

# combine to mp4
ffmpeg -framerate 25 -i $EX/frames/frame_%04d.ppm -pix_fmt yuv420p -y $EX/timelapse_out.mp4

echo "Timelapse written to $EX/timelapse_out.mp4"
# Build from repository root with OpenMP and float precision
mkdir -p build
pushd build
cmake -DUSE_OPENMP=ON -DUSE_FLOAT=ON ..
cmake --build . --target units_pixel_timelapse_core
popd

# Run core-backed example
./build/examples/pixel_timelapse/units_pixel_timelapse_core --width 256 --height 256 --steps 100 --seed 42

# Combine to mp4
ffmpeg -framerate 25 -i examples/pixel_timelapse/frames_png_core/frame_%04d.png -pix_fmt yuv420p -y examples/pixel_timelapse/timelapse_colored_core.mp4

echo "Timelapse written to examples/pixel_timelapse/timelapse_colored_core.mp4"
