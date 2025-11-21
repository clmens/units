# Pixel Timelapse Example

This example simulates a grid of Units wired as a torus (wrap-around neighbors) and
writes one frame per timestep. Two versions are available:

1. **units_pixel_timelapse** - Original version using Unit/Net classes, outputs PPM frames
2. **units_pixel_timelapse_core** - High-performance version using UnitsCore, outputs colored PNG frames

Both support CLI options to set grid size, steps, and RNG seed for reproducibility.

## Build

From repository root:
```bash
mkdir build && cd build
cmake ..
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
# default: 100x100, 200 steps
./units_pixel_timelapse_core

# customize: 256x256, 100 steps, fixed seed
./units_pixel_timelapse_core --width 256 --height 256 --steps 100 --seed 42
```

### Original version
```bash
# default: 100x100, 200 steps
./units_pixel_timelapse

# customize: 100x100, 300 steps, fixed seed
./units_pixel_timelapse --width 100 --height 100 --steps 300 --seed 12345
```

## Create timelapse video

### Core version (colored PNG output)
```bash
ffmpeg -framerate 25 -i examples/pixel_timelapse/frames_png_core/frame_%04d.png -pix_fmt yuv420p -y timelapse_colored_core.mp4
```

### Original version (grayscale PPM output)
```bash
ffmpeg -framerate 25 -i examples/pixel_timelapse/frames/frame_%04d.ppm -pix_fmt yuv420p -y timelapse_out.mp4
```

## Notes
- The core version uses UnitsCore for significantly better performance on large grids (e.g., 256x256+)
- The core version outputs colored PNG frames (red=positive, blue=negative values)
- The original version compiles Unit.cpp/Net.cpp from the repository root (no Cinder dependency required)
- If Unit.h is missing `#include <vector>`, this has been fixed in the current version

