# Realtime Viewer

Interactive visualization for Units simulation using SDL2 and OpenGL.

## Features

- **GPU-accelerated rendering**: Uses OpenGL shaders for color mapping when `USE_GPU_COLORMAP=ON`
- **High-resolution support**: Efficient rendering for grids up to 4K+ resolution
- **Realtime simulation**: Run simulation and visualization at configurable frame rates
- **Pixel buffer objects**: Asynchronous texture uploads for better performance

## Building

The realtime viewer requires SDL2. When SDL2 is available, it will be built automatically.

### With GPU colormap (recommended):

```bash
cmake -DUSE_GPU_COLORMAP=ON ..
cmake --build .
```

### CPU-only mode:

```bash
cmake -DUSE_GPU_COLORMAP=OFF ..
cmake --build .
```

## Usage

```bash
./units_realtime_viewer [OPTIONS]
```

Options:
- `--width W` - Grid width (default: 512)
- `--height H` - Grid height (default: 512)
- `--scale S` - Window scale factor (default: 1)
- `--fps F` - Target frame rate (default: 30)
- `--scenario S` - Initial scenario: "random" (default)

## Controls

- **ESC** or close window - Exit viewer

## Performance Tips

- For high resolutions (1024x1024+), use GPU colormap mode
- Lower FPS for very large grids to maintain interactivity
- Use scale=1 for best performance (no upscaling)
- Build with `USE_FLOAT=ON` and `USE_OPENMP=ON` for faster simulation
