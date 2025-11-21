#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <cmath>
#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "units_core.h"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    int width = 100;
    int height = 100;
    int steps = 200;
    unsigned long long seed = std::random_device{}();
    std::string outdir = "examples/pixel_timelapse/frames_png_core";

    // simple CLI parsing
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--width" && i+1 < argc) width = std::stoi(argv[++i]);
        else if (a == "--height" && i+1 < argc) height = std::stoi(argv[++i]);
        else if (a == "--steps" && i+1 < argc) steps = std::stoi(argv[++i]);
        else if (a == "--seed" && i+1 < argc) seed = std::stoull(argv[++i]);
        else if (a == "--outdir" && i+1 < argc) outdir = argv[++i];
    }

    fs::create_directories(outdir);

    // Create UnitsCore simulation with torus topology
    UnitsCore core(width, height, 1.0, true);

    // Initialize with random values
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<units_real> dist(-1.0, 1.0);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            core.set_value(x, y, dist(rng));
        }
    }

    for (int step = 0; step < steps; ++step) {
        // Run one simulation step
        core.step();

        // Get values for visualization
        const auto& values = core.values();

        // Normalize and convert to RGB for colorful output
        units_real maxv = 0.0;
        for (const auto& v : values) {
            maxv = std::max(maxv, std::abs(v));
        }
        if (maxv == 0.0) maxv = 1.0;

        std::vector<uint8_t> rgb(width * height * 3);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                std::size_t idx = y * width + x;
                units_real v = values[idx];
                units_real n = (v / maxv) * 0.5 + 0.5;
                n = std::min(static_cast<units_real>(1.0), std::max(static_cast<units_real>(0.0), n));
                
                // Create colored output: blue for negative, red for positive
                uint8_t r = 0, g = 0, b = 0;
                if (v > 0) {
                    r = static_cast<uint8_t>(std::round(n * 255.0));
                    g = static_cast<uint8_t>(std::round(n * 128.0));
                } else {
                    b = static_cast<uint8_t>(std::round(n * 255.0));
                    g = static_cast<uint8_t>(std::round(n * 128.0));
                }
                
                std::size_t off = idx * 3;
                rgb[off + 0] = r;
                rgb[off + 1] = g;
                rgb[off + 2] = b;
            }
        }

        std::ostringstream name;
        name << outdir << "/frame_" << std::setw(4) << std::setfill('0') << step << ".png";
        stbi_write_png(name.str().c_str(), width, height, 3, rgb.data(), width * 3);
    }

    std::cout << "Wrote " << steps << " PNG frames to " << outdir << " (seed=" << seed << ")\n";
    std::cout << "Use: ffmpeg -framerate 25 -i " << outdir << "/frame_%04d.png -pix_fmt yuv420p -y timelapse_colored_core.mp4\n";
    return 0;
}
