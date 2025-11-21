# Units - High-Performance Simulation Framework

A high-performance Units simulation framework optimized for large grids (>= 1024x1024) with GPU-accelerated visualization.

## Features

- **UnitsCore**: Lightweight, cache-friendly simulation core with flat arrays and optimized neighbor indexing
- **Per-thread Accumulators**: Lock-free parallel push implementation avoiding atomic operations
- **GPU Color Mapping**: OpenGL shader-based color mapping with asynchronous texture upload
- **Microbenchmark**: Command-line benchmark tool for performance testing
- **Realtime Viewer**: SDL/OpenGL-based visualization with configurable resolution and frame rate

## Building

### Basic Build

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `USE_OPENMP` | OFF | Enable OpenMP parallelization |
| `USE_FLOAT` | OFF | Use float instead of double (reduces memory for large grids) |
| `USE_PER_THREAD_ACCUM` | OFF | Enable per-thread accumulators (requires OpenMP) |
| `USE_GPU_COLORMAP` | ON | Enable GPU color mapping in viewer (requires OpenGL) |
| `USE_SIMD` | OFF | Enable SIMD optimizations (-march=native) |

### Recommended Configurations

#### Local Desktop - AMD Radeon 7900 XT + High-End CPU

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DUSE_OPENMP=ON \
      -DUSE_FLOAT=ON \
      -DUSE_GPU_COLORMAP=ON \
      -DUSE_SIMD=ON \
      ..
```

**Rationale**: GPU color mapping maximizes upload throughput with persistent PBOs. Float buffers reduce memory bandwidth. SIMD helps CPU-heavy simulations.

**Usage**: High-resolution rendering (1024x1024+) with GPU-accelerated display. Use smaller step counts if GPU-bound.

#### Local Desktop - Intel 9800X3D (Many Cores)

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DUSE_OPENMP=ON \
      -DUSE_FLOAT=ON \
      -DUSE_PER_THREAD_ACCUM=ON \
      -DUSE_GPU_COLORMAP=ON \
      -DUSE_SIMD=ON \
      ..
```

**Rationale**: Per-thread accumulators avoid atomic contention on many-core CPUs. GPU color mapping still helps even if CPU-heavy.

**Usage**: Prefer larger grids (512x512+) where per-thread accumulators outweigh merge overhead.

#### Cloud Environments

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DUSE_OPENMP=ON \
      -DUSE_FLOAT=ON \
      ..
```

**Rationale**: Cloud instances may lack GPU or have limited OpenGL. Use CPU path for portability.

**Usage**: Run benchmarks with the workflow. For GPU cloud instances, enable `-DUSE_GPU_COLORMAP=ON` if OpenGL/Vulkan available.

## Running the Benchmark

```bash
# After building
./bench/bench_units --width 512 --height 512 --steps 200

# Example output:
# {"width":512,"height":512,"steps":200,"time_s":2.345,"steps_per_s":85.31,"use_per_thread_accum":true}
```

### Benchmark Options

- `--width W`: Grid width (default: 128)
- `--height H`: Grid height (default: 128)
- `--steps S`: Number of simulation steps (default: 100)
- `--warmup W`: Number of warmup steps (default: 5)
- `--seed SEED`: Random seed
- `--use_per_thread_accum [0|1]`: Override compile-time flag for reporting

### CI Benchmark

Benchmark workflow runs automatically on push/PR to `perf-optimizations` or `ki` branches. Results are saved as artifacts.

Manual trigger:
- Go to Actions → Benchmark → Run workflow

## Running the Realtime Viewer

```bash
# After building (requires SDL2 and optionally OpenGL)
./examples/realtime_viewer/units_realtime_viewer --width 1024 --height 1024 --scale 1 --fps 30
```

### Viewer Options

- `--width W`: Grid width (default: 512)
- `--height H`: Grid height (default: 512)
- `--scale S`: Window scale factor (default: 1)
- `--fps F`: Target frame rate (default: 30)
- `--scenario S`: Initial scenario: "random" or "center" (default: random)

### Controls

- **ESC** or **Q**: Quit
- Window close button: Quit

### GPU Color Mapping

When `USE_GPU_COLORMAP=ON` (default if OpenGL available):
- Float values uploaded to GPU as R32F texture
- Color mapping done in fragment shader (HSV colormap)
- Uses Pixel Buffer Objects (PBO) for asynchronous upload
- Significantly faster for high-resolution rendering (1024x1024+)

When `USE_GPU_COLORMAP=OFF`:
- CPU converts float values to RGB on host
- Uses `SDL_UpdateTexture` for upload
- Suitable for environments without OpenGL

## Performance Notes

### Per-Thread Accumulators

**When to use**: Large grids (>= 512x512), many CPU cores, low-degree graphs

**Trade-offs**:
- **Pro**: Avoids atomic operations, scales better on many-core CPUs
- **Con**: Increases memory bandwidth due to merge step
- **Best for**: >= 8 cores, grid size >= 512x512

**When to avoid**: Small grids (< 64x64), few cores (< 4)

### Float vs Double

**USE_FLOAT=ON** (recommended for >= 1024x1024):
- Reduces memory footprint by 50%
- Reduces memory bandwidth
- May lose precision for very long simulations

**USE_FLOAT=OFF** (default):
- Higher precision
- More memory bandwidth
- Suitable for smaller grids or precision-critical scenarios

### GPU Color Mapping

**Benefits**:
- Offloads color mapping to GPU
- Essential for realtime display at 4K+ resolutions
- Reduces CPU time per frame

**Requirements**:
- SDL2
- OpenGL 3.3+ (Core profile)
- GPU with shader support

## Examples

### Pixel Timelapse

Generates a sequence of PPM frames:

```bash
./examples/pixel_timelapse/units_pixel_timelapse --width 256 --height 256 --steps 500 --outdir frames
```

### Console App

Simple console output:

```bash
./examples/console_app/units_console
```

## License

See [LICENSE](LICENSE) for details.
