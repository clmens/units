# Units

A high-performance cellular simulation engine optimized for large grids with configurable precision and parallelization strategies.

## Features

- **Cache-friendly core**: Flat arrays and integer neighbor indices for efficient memory access
- **Flexible precision**: Choose between `float` (USE_FLOAT) and `double` for real_t
- **OpenMP parallelization**: Multi-threaded simulation with configurable accumulation strategies
- **Per-thread accumulators**: Source-centric push algorithm that eliminates atomic operations for large grids
- **GPU-accelerated viewer**: Optional OpenGL-based colormap rendering for real-time visualization
- **Comprehensive benchmarking**: CLI tool with JSON output for performance analysis

## Building

### Prerequisites

- CMake 3.15+
- C++17 compatible compiler
- Optional: OpenMP-capable compiler for parallelization
- Optional: SDL2 and OpenGL for realtime_viewer

### Build Options

The following CMake options control performance and features:

| Option | Default | Description |
|--------|---------|-------------|
| `USE_OPENMP` | OFF | Enable OpenMP parallelization |
| `USE_FLOAT` | OFF | Use float instead of double for real_t |
| `USE_PER_THREAD_ACCUM` | OFF | Use per-thread accumulators (requires OpenMP) |
| `USE_SIMD` | OFF | Enable SIMD optimizations (experimental) |
| `USE_GPU_COLORMAP` | ON | Enable GPU colormap in realtime_viewer (requires OpenGL) |

### Quick Start

```bash
# Basic build
mkdir build && cd build
cmake ..
make -j

# Performance build with all optimizations
cmake -DCMAKE_BUILD_TYPE=Release \
      -DUSE_OPENMP=ON \
      -DUSE_FLOAT=ON \
      -DUSE_PER_THREAD_ACCUM=ON \
      ..
make -j
```

## Running Benchmarks

The `bench_units` executable provides performance benchmarking:

```bash
# Basic benchmark
./build/bench/bench_units --width 512 --height 512 --steps 100

# Full benchmark options
./build/bench/bench_units \
  --width 1024 \
  --height 1024 \
  --steps 50 \
  --warmup 5 \
  --seed 12345
```

Output is a single JSON line with performance metrics:
```json
{"width": 512, "height": 512, "steps": 100, "time_s": 0.123, "steps_per_s": 812.3, "use_per_thread_accum": true, "threads": 16, "precision": "float"}
```

## Performance Tuning

### Per-Thread Accumulator Strategy

The `USE_PER_THREAD_ACCUM` option enables a source-centric push algorithm that trades memory for speed:

**When to use:**
- Large grids (1024x1024 or larger)
- Many CPU threads (8+)
- Memory is not constrained (~64MB for 1024x1024 with 16 threads and float precision)

**When NOT to use:**
- Small grids (<512x512)
- Few CPU threads (<4)
- Memory-constrained environments

The default destination-centric algorithm uses atomic operations but has better cache locality and lower memory usage.

### Recommended Configurations

#### AMD Radeon 7900 XT + Intel Core i9-9800X3D (Local Development)

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DUSE_OPENMP=ON \
      -DUSE_FLOAT=ON \
      -DUSE_PER_THREAD_ACCUM=ON \
      -DUSE_GPU_COLORMAP=ON \
      ..
```

Expected performance: ~2000-5000 steps/sec for 1024x1024 grid with 16+ threads.

#### Cloud/CI Environments (Ubuntu-latest, 2-4 cores)

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DUSE_OPENMP=ON \
      -DUSE_FLOAT=ON \
      ..
```

Note: Skip `USE_PER_THREAD_ACCUM` on low core count systems as the atomic path will be faster.

### Float vs Double Precision

- `USE_FLOAT=ON`: ~2x faster, sufficient for most visual simulations
- `USE_FLOAT=OFF` (double): Better numerical accuracy for scientific applications

## Realtime Viewer

If SDL2 is detected, the realtime viewer will be built:

```bash
./build/examples/realtime_viewer/realtime_viewer \
  --width 512 \
  --height 512 \
  --scale 2 \
  --fps 60 \
  --scenario 1  # 0=random, 1=center, 2=edges
```

Press ESC to exit.

### GPU Colormap

When `USE_GPU_COLORMAP=ON` and OpenGL is available:
- Uploads simulation data as R32F texture via PBO for async transfer
- Renders using OpenGL fixed-function pipeline (compatible with GL 2.1+)
- Significantly faster for large grids (512x512+)

CPU fallback is automatically used when OpenGL is not available.

## Troubleshooting

### Build Issues

**OpenMP not found:**
```
sudo apt-get install libomp-dev  # Ubuntu/Debian
brew install libomp              # macOS
```

**SDL2 not found:**
```
sudo apt-get install libsdl2-dev  # Ubuntu/Debian
brew install sdl2                 # macOS
```

### Performance Issues

**Low steps/sec:**
- Ensure `CMAKE_BUILD_TYPE=Release`
- Enable OpenMP: `-DUSE_OPENMP=ON`
- Try float precision: `-DUSE_FLOAT=ON`
- For large grids with 8+ threads: `-DUSE_PER_THREAD_ACCUM=ON`

**Unexpected results:**
- Check thread count: `export OMP_NUM_THREADS=16`
- Disable CPU frequency scaling (Linux): `sudo cpupower frequency-set -g performance`

## Expected Outputs

### Benchmark Output

For a 512x512 grid with 100 steps on a modern 8-core CPU:
- Without optimizations: ~100-300 steps/sec
- With OpenMP + float: ~500-1500 steps/sec
- With OpenMP + float + per-thread accum: ~800-2000 steps/sec (on 8+ threads)

### Viewer Output

The viewer should display smooth animation at target FPS (default 30). If you see stuttering:
- Reduce grid size: `--width 256 --height 256`
- Lower target FPS: `--fps 15`
- Try GPU colormap: Build with `-DUSE_GPU_COLORMAP=ON`

## CI/CD

The benchmark workflow runs automatically on pushes to `ki` branch. It tests multiple configurations and uploads JSON artifacts for performance tracking.

See `.github/workflows/benchmark.yml` for details.

## License

See LICENSE file.
