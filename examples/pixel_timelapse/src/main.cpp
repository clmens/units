#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <cmath>
#include <filesystem>
#include "../../Unit.h"

namespace fs = std::filesystem;

static void write_ppm(const fs::path &path, int w, int h, const std::vector<uint8_t> &rgb) {
    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << w << " " << h << "\n255\n";
    out.write(reinterpret_cast<const char*>(rgb.data()), rgb.size());
}

int main(int argc, char** argv) {
    int width = 100;
    int height = 100;
    int steps = 200;
    unsigned long long seed = std::random_device{}();
    std::string outdir = "examples/pixel_timelapse/frames";

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

    using UnitPtr = std::shared_ptr<Unit>;
    std::vector<UnitPtr> grid;
    grid.reserve(width * height);

    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) grid.push_back(std::make_shared<Unit>(x, y, 0.0));

    auto idx = [width](int x, int y) { return y * width + x; };

    // Torus connectivity
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            auto u = grid[idx(x,y)];
            for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;
                int nx = (x + dx + width) % width;
                int ny = (y + dy + height) % height;
                u->connections.push_back(grid[idx(nx, ny)]);
            }
        }
    }

    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    for (auto &u : grid) u->set_value(dist(rng));

    for (int step = 0; step < steps; ++step) {
        // update and push
        for (auto &u : grid) u->update();
        for (auto &u : grid) u->push();

        // normalization
        double maxv = 0.0;
        for (auto &u : grid) maxv = std::max(maxv, std::abs(u->m_value));
        if (maxv == 0.0) maxv = 1.0;

        std::vector<uint8_t> rgb(width * height * 3);
        for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
            double v = grid[idx(x,y)]->m_value;
            double n = (v / maxv) * 0.5 + 0.5;
            n = std::min(1.0, std::max(0.0, n));
            uint8_t b = static_cast<uint8_t>(std::round(n * 255.0));
            size_t off = (y * width + x) * 3;
            rgb[off + 0] = b; rgb[off + 1] = b; rgb[off + 2] = b;
        }

        std::ostringstream name;
        name << "frame_" << std::setw(4) << std::setfill('0') << step << ".ppm";
        write_ppm(fs::path(outdir) / name.str(), width, height, rgb);
    }

    std::cout << "Wrote " << steps << " frames to " << outdir << " (seed=" << seed << ")\n";
    std::cout << "Use: ffmpeg -framerate 25 -i " << outdir << "/frame_%04d.ppm -pix_fmt yuv420p -y out.mp4\n";
    return 0;
}