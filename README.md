# Units

A high-performance cellular simulation engine optimized for large grids with configurable precision and parallelization strategies.

## Repository Purpose

This repository serves as a **high-performance reference implementation** and **performance playground** for the Units cellular simulation engine. It focuses on:

- **Optimized core engine**: Fast, cache-friendly cellular automata with configurable precision and parallelization
- **Comprehensive benchmarking**: Performance testing and optimization research
- **Real-time visualization**: GPU-accelerated viewer for interactive exploration

Future artistic and experimental "brain" directions may be explored in separate repositories (e.g., `netted-units`), while this repository remains the canonical performance reference and engine implementation.

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

### Windows with MinGW (Recommended)

For Windows users, we provide a simple build script that uses MinGW and CMake:

```bash
# Using the provided build script
build_windows_mingw.bat

# Or manually:
mkdir build_mingw
cd build_mingw
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel 4
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
- When using large grids (e.g., 1024x1024+) with many threads
- When you have sufficient memory for per-thread accumulators
- For better performance when the simulation has many active cells

**Implementation Details:**
- Each thread maintains its own accumulator array
- Reduces memory contention and improves cache locality
- Eliminates atomic operations for large grid simulations
- Memory usage scales with number of threads

### SIMD Optimization

The `USE_SIMD` option enables experimental SIMD (Single Instruction, Multiple Data) optimizations:

**When to use:**
- When targeting modern CPUs with SIMD support (SSE, AVX)
- For maximum performance on compatible hardware
- When compiling with optimizations enabled

## Examples

- `console_app` - A simple console-based example
- `pixel_timelapse` - A pixel-based timelapse visualization
- `realtime_viewer` - A real-time visualization with SDL2

## Documentation

See [docs/](docs/) for more information.
