# Pixel Timelapse Example (Torus, random init)

This example simulates a grid of Units wired as a torus (wrap-around neighbors) and
writes one PPM frame per timestep. It supports CLI options to set grid size, steps,
and RNG seed for reproducibility.

Build
```
cd examples/pixel_timelapse
mkdir build && cd build
cmake ..
cmake --build .
```

Run
```
# default: 100x100, 200 steps
./units_pixel_timelapse

# customize: 100x100, 300 steps, fixed seed
./units_pixel_timelapse --width 100 --height 100 --steps 300 --seed 12345
```

Create timelapse video
```
ffmpeg -framerate 25 -i examples/pixel_timelapse/frames/frame_%04d.ppm -pix_fmt yuv420p -y timelapse_out.mp4
```

Notes
- The example compiles Unit.cpp/Net.cpp from the repository root so no Cinder dependency is required.
- If Unit.h is missing <vector>, add `#include <vector>` near the top of Unit.h so the example compiles.

---

## Performance Benchmarking

This repository includes a microbenchmark tool for testing UnitsCore performance on large grids.

### Building the Benchmark

From the repository root:

```bash
mkdir build && cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release -DUSE_OPENMP=ON -DUSE_FLOAT=ON -DUSE_PER_THREAD_ACCUM=ON
cmake --build . -j
```

### Running the Benchmark

```bash
# Small grid (128x128)
./bench/bench_units --width 128 --height 128 --steps 500

# Medium grid (512x512)
./bench/bench_units --width 512 --height 512 --steps 200

# Large grid (1024x1024)
./bench/bench_units --width 1024 --height 1024 --steps 50
```

### Interpreting Results

The benchmark outputs JSON with the following fields:
- `width`, `height`: Grid dimensions
- `steps`: Number of simulation steps executed
- `time_s`: Total execution time in seconds
- `steps_per_s`: Throughput (steps per second)
- `cells`: Total number of grid cells
- `use_float`: Whether float was used instead of double
- `use_openmp`: Whether OpenMP parallelization was enabled
- `use_per_thread_accum`: Whether per-thread accumulators were used

### Build Flags for Large Grids

For optimal performance on large grids (1024x1024+):

- **USE_FLOAT=ON**: Reduces memory bandwidth and cache pressure (recommended for grids > 512x512)
- **USE_OPENMP=ON**: Enables multi-threaded parallelization
- **USE_PER_THREAD_ACCUM=ON**: Uses per-thread local accumulators instead of atomic operations (beneficial for 8+ threads on large grids)

Example:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_OPENMP=ON -DUSE_FLOAT=ON -DUSE_PER_THREAD_ACCUM=ON
```

### Performance Tips

1. **For grids < 512x512**: Use default settings (double precision, destination-centric push with atomics)
2. **For grids 512x512 to 1024x1024**: Enable `USE_FLOAT=ON` and `USE_OPENMP=ON`
3. **For grids > 1024x1024**: Enable all optimizations including `USE_PER_THREAD_ACCUM=ON`
4. Always use **Release** build type for benchmarking
5. Run with multiple steps (100+) to get accurate steady-state performance

