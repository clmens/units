# Pixel Timelapse Example

This example simulates a grid of Units wired as a torus (wrap-around neighbors) and
writes one frame per timestep. Two versions are available:

1. **units_pixel_timelapse** - Original version using Unit/Net classes, outputs PPM frames (requires Cinder dependencies)
2. **units_pixel_timelapse_core** - High-performance version using UnitsCore, outputs colored PNG frames (recommended)

Both support CLI options to set grid size, steps, and RNG seed for reproducibility.

## Quick Start

The easiest way to generate timelapse videos is using the provided script:

```bash
# From repository root, run the timelapse script
bash scripts/make_timelapse.sh

# Or with custom parameters: WIDTH HEIGHT STEPS SEED
bash scripts/make_timelapse.sh 256 256 200 42
```

This script will:
- Build the core PNG timelapse example with optimizations (OpenMP, float precision)
- Run the simulation and generate frames
- Create the timelapse video at `examples/pixel_timelapse/timelapse_colored_core.mp4`
- Optionally build and run the original PPM version if Cinder dependencies are available

## Build

From repository root:
```bash
mkdir build && cd build
cmake -DUSE_OPENMP=ON -DUSE_FLOAT=ON ..
cmake --build . --target units_pixel_timelapse_core
```

Or from this directory:
```bash
mkdir build && cd build
cmake ../../..
cmake --build . --target units_pixel_timelapse_core
```

### Build Options

- `-DUSE_OPENMP=ON` - Enable OpenMP parallelism for faster simulation
- `-DUSE_FLOAT=ON` - Use float precision instead of double for better cache performance

Example with options:
```bash
cmake -DUSE_OPENMP=ON -DUSE_FLOAT=ON ..
cmake --build . --target units_pixel_timelapse_core
```

## Run

### Core version (recommended for large grids)
```bash
# From repository root, default: 100x100, 200 steps
./build/examples/pixel_timelapse/units_pixel_timelapse_core

# Customize: 256x256, 100 steps, fixed seed
./build/examples/pixel_timelapse/units_pixel_timelapse_core --width 256 --height 256 --steps 100 --seed 42
```

### Original version (requires Cinder)
```bash
# From examples/pixel_timelapse/build, default: 100x100, 200 steps
./units_pixel_timelapse

# Customize: 100x100, 300 steps, fixed seed
./units_pixel_timelapse --width 100 --height 100 --steps 300 --seed 12345
```

## Create timelapse video

### Core version (colored PNG output)
```bash
# From repository root
ffmpeg -framerate 25 -i examples/pixel_timelapse/frames_png_core/frame_%04d.png -pix_fmt yuv420p -y examples/pixel_timelapse/timelapse_colored_core.mp4
```

Output: `examples/pixel_timelapse/timelapse_colored_core.mp4`

### Original version (grayscale PPM output)
```bash
# From repository root
ffmpeg -framerate 25 -i examples/pixel_timelapse/frames/frame_%04d.ppm -pix_fmt yuv420p -y examples/pixel_timelapse/timelapse_out.mp4
```

Output: `examples/pixel_timelapse/timelapse_out.mp4`

## Notes
- The core version uses UnitsCore for significantly better performance on large grids (e.g., 256x256+)
- The core version outputs colored PNG frames (red=positive, blue=negative values)
- The original version compiles Unit.cpp/Net.cpp from the repository root (requires Cinder dependency)
- The script `scripts/make_timelapse.sh` handles all build and video generation steps automatically