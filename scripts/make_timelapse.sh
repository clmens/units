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