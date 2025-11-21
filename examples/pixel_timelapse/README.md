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
