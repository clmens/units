# Units - High-Performance Grid Simulation

A high-performance grid simulation framework with GPU-accelerated visualization.

## Features

- **UnitsCore**: Optimized flat-array simulation engine for large grids (>=1024x1024)
- **Flexible precision**: Float or double precision support via `USE_FLOAT`
- **Per-thread accumulators**: Avoid atomic operations for better multi-core scaling
- **GPU color mapping**: OpenGL shader-based visualization for high-resolution grids
- **Microbenchmark**: Performance measurement tools for different configurations
- **CI benchmarks**: Automated performance tracking

## Building

### Basic build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Performance-optimized build

For maximum performance on large grids:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DUSE_OPENMP=ON \
      -DUSE_FLOAT=ON \
      -DUSE_PER_THREAD_ACCUM=ON \
      -DUSE_SIMD=ON \
      -DUSE_GPU_COLORMAP=ON \
      ..
cmake --build . -j
```

### Build options

- `USE_OPENMP` (default: OFF) - Enable OpenMP parallelization
- `USE_FLOAT` (default: OFF) - Use float instead of double for reduced memory bandwidth
- `USE_PER_THREAD_ACCUM` (default: OFF) - Use per-thread accumulators to avoid atomics (requires OpenMP)
- `USE_SIMD` (default: OFF) - Enable SIMD optimizations (-march=native)
- `USE_GPU_COLORMAP` (default: ON) - Enable GPU-accelerated color mapping in viewer (requires OpenGL)
- `BUILD_EXAMPLES` (default: ON) - Build example applications
- `BUILD_BENCH` (default: ON) - Build benchmark tools

## Running Benchmarks

### Local benchmarks

```bash
cd build
./bench/bench_units --width 512 --height 512 --steps 200
```

Output is JSON format:
```json
{"width":512,"height":512,"steps":200,"time_s":1.234,"steps_per_s":162.07,"use_per_thread_accum":true}
```

### CI benchmarks

The benchmark workflow runs automatically on pushes to `perf-optimizations` and `ki` branches, testing:
- 128x128 grid, 500 steps
- 512x512 grid, 200 steps  
- 1024x1024 grid, 50 steps

Results are uploaded as artifacts and displayed in a summary table.

## Running the Realtime Viewer

The realtime viewer provides interactive visualization of the simulation.

```bash
cd build
./examples/realtime_viewer/units_realtime_viewer --width 1024 --height 1024 --scale 1 --fps 30
```

Options:
- `--width W` - Grid width (default: 512)
- `--height H` - Grid height (default: 512)
- `--scale S` - Window scale factor (default: 1)
- `--fps F` - Target frame rate (default: 30)
- `--scenario S` - Initial scenario (default: "random")

Press ESC or close the window to exit.

## Performance Tuning

### Local Desktop - AMD Radeon 7900 XT

For maximum performance with a high-end GPU:

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DUSE_OPENMP=ON \
      -DUSE_FLOAT=ON \
      -DUSE_GPU_COLORMAP=ON \
      -DUSE_SIMD=ON \
      ..
```

**Recommendations:**
- Use GPU color mapping (`USE_GPU_COLORMAP=ON`) to offload visualization work
- Enable float precision (`USE_FLOAT=ON`) to reduce memory bandwidth
- OpenMP helps for multi-threaded simulation
- PBO mapping provides asynchronous texture uploads for better frame rates
- For very high resolutions (4K+), the GPU shader path is essential

### Local Desktop - Intel 9800X3D (Many-core CPU)

For CPU-heavy simulation on many-core processors:

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DUSE_OPENMP=ON \
      -DUSE_FLOAT=ON \
      -DUSE_PER_THREAD_ACCUM=ON \
      -DUSE_SIMD=ON \
      -DUSE_GPU_COLORMAP=ON \
      ..
```

**Recommendations:**
- Enable per-thread accumulators (`USE_PER_THREAD_ACCUM=ON`) to avoid atomic contention on many cores
- Use SIMD optimizations (`USE_SIMD=ON`) for vectorized operations
- Float precision reduces memory bandwidth, which is critical for large grids
- GPU color mapping still helps reduce CPU work for visualization even with strong CPU

### Cloud Environments

For cloud instances with GPU acceleration:

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DUSE_OPENMP=ON \
      -DUSE_FLOAT=ON \
      -DUSE_GPU_COLORMAP=ON \
      ..
```

**Recommendations:**
- Use `USE_FLOAT=ON` to reduce memory usage and bandwidth
- Enable OpenMP for multi-core cloud instances
- GPU color mapping and texture uploads via OpenGL/Vulkan where available
- For headless benchmarking, disable examples and just run `bench_units`

### Performance Trade-offs

**Per-thread accumulators** (`USE_PER_THREAD_ACCUM`):
- **Pros**: Eliminates atomic operations, better scaling on many-core CPUs
- **Cons**: Increased memory bandwidth for merge step, may lose on small grids (<64x64)
- **Best for**: Large grids (>=512x512), many cores (8+), low-degree graphs

**Float precision** (`USE_FLOAT`):
- **Pros**: 2x less memory, 2x less bandwidth, better cache utilization
- **Cons**: Reduced numerical precision (usually acceptable for visualization)
- **Best for**: Very large grids (>=1024x1024) where memory bandwidth is bottleneck

**GPU color mapping** (`USE_GPU_COLORMAP`):
- **Pros**: Offloads CPU work, essential for high-res realtime (4K+), faster frame rates
- **Cons**: Requires OpenGL support
- **Best for**: Realtime visualization at high resolutions

## Testing

Build and run the console example:
```bash
cd build
./examples/console_app/units_console
```

Build and run the pixel timelapse example:
```bash
cd build
./examples/pixel_timelapse/units_pixel_timelapse --width 100 --height 100 --steps 50
```

## License

See LICENSE file for details.
