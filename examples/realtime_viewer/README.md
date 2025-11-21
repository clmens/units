# Realtime Viewer

This example provides a real-time SDL2-based visualization of the UnitsCore simulation. It displays the grid values as grayscale intensities and updates at a configurable frame rate.

## Prerequisites

- SDL2 library installed
- On Ubuntu/Debian: `sudo apt-get install libsdl2-dev`
- On macOS: `brew install sdl2`

## Build

From the repository root:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Or from this directory:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Run

```bash
# Default: 256x256 grid, scale=2, 30 FPS, random initialization
./realtime_viewer

# Custom grid size and scale
./realtime_viewer --width 512 --height 512 --scale 1

# Center stimulus scenario
./realtime_viewer --scenario 1

# Edge stimulus scenario
./realtime_viewer --scenario 2 --fps 60
```

## Options

- `--width <W>`: Grid width (default: 256)
- `--height <H>`: Grid height (default: 256)
- `--scale <S>`: Pixel scale factor (default: 2)
- `--fps <F>`: Target frame rate (default: 30)
- `--scenario <N>`: Initial scenario
  - 0: Random values (default)
  - 1: Center stimulus
  - 2: Edge stimulus
- `--help`: Show help message

## Controls

- **ESC** or close window to exit

## Notes

- The viewer uses UnitsCore for simulation, providing optimized performance for large grids
- Values are normalized to [0, 255] range for display each frame
- The simulation runs continuously at the target frame rate
