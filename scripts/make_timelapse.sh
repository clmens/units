#!/usr/bin/env bash
# Timelapse generation script for the Units repository
# This script builds and runs both PPM and PNG timelapse examples and creates videos
set -euo pipefail

# Determine repository root (script is in ROOT_DIR/scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
EX="${ROOT_DIR}/examples/pixel_timelapse"

# Default parameters (can be overridden by command line args)
WIDTH=${1:-128}
HEIGHT=${2:-128}
STEPS=${3:-100}
SEED=${4:-42}

echo "================================================"
echo "Units Timelapse Generation"
echo "Resolution: ${WIDTH}x${HEIGHT}, Steps: ${STEPS}, Seed: ${SEED}"
echo "Repository root: ${ROOT_DIR}"
echo "================================================"

cd "${ROOT_DIR}"

# ===== Build Phase =====
echo ""
echo "Building from repository root with OpenMP and float precision..."
mkdir -p build
cd build
cmake -DUSE_OPENMP=ON -DUSE_FLOAT=ON ..
cmake --build . --target units_pixel_timelapse_core -j$(nproc)
cd "${ROOT_DIR}"

# ===== Core PNG Version =====
echo ""
echo "Running core PNG timelapse example..."
./build/examples/pixel_timelapse/units_pixel_timelapse_core \
    --width ${WIDTH} --height ${HEIGHT} --steps ${STEPS} --seed ${SEED}

echo "Creating core PNG timelapse video..."
ffmpeg -y -framerate 25 \
    -i examples/pixel_timelapse/frames_png_core/frame_%04d.png \
    -pix_fmt yuv420p \
    examples/pixel_timelapse/timelapse_colored_core.mp4

echo "Core PNG timelapse written to: ${EX}/timelapse_colored_core.mp4"

# ===== Original PPM Version (if buildable) =====
# Note: The original PPM version requires Cinder dependencies which may not be available
# We attempt to build it, but gracefully skip if it fails
echo ""
echo "Attempting to build original PPM timelapse example..."
if cd "${EX}" && mkdir -p build && cd build && cmake .. && cmake --build . --target units_pixel_timelapse 2>/dev/null; then
    cd "${ROOT_DIR}"
    echo "Running original PPM timelapse example..."
    ${EX}/build/units_pixel_timelapse \
        --width ${WIDTH} --height ${HEIGHT} --steps ${STEPS} --seed ${SEED}
    
    echo "Creating original PPM timelapse video..."
    ffmpeg -y -framerate 25 \
        -i ${EX}/frames/frame_%04d.ppm \
        -pix_fmt yuv420p \
        ${EX}/timelapse_out.mp4
    
    echo "Original PPM timelapse written to: ${EX}/timelapse_out.mp4"
else
    cd "${ROOT_DIR}"
    echo "WARNING: Could not build original PPM example (requires Cinder dependencies)"
    echo "         Skipping PPM timelapse generation. Core PNG version is complete."
fi

echo ""
echo "================================================"
echo "Timelapse generation complete!"
echo "Core PNG video: ${EX}/timelapse_colored_core.mp4"
if [ -f "${EX}/timelapse_out.mp4" ]; then
    echo "Original PPM video: ${EX}/timelapse_out.mp4"
fi
echo "================================================"
