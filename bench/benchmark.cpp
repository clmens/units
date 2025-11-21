#include <iostream>
#include <chrono>
#include <random>
#include <string>
#include <cstring>
#include "../src/units_core.h"

// Precision type based on USE_FLOAT macro
#ifdef USE_FLOAT
using ValueType = float;
#else
using ValueType = double;
#endif

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " [options]\n"
              << "  --width W         Grid width (default: 128)\n"
              << "  --height H        Grid height (default: 128)\n"
              << "  --steps S         Number of simulation steps (default: 100)\n"
              << "  --warmup W        Number of warmup steps (default: 5)\n"
              << "  --seed SEED       Random seed (default: random)\n"
              << "  --use_per_thread_accum [0|1]  Report flag (default: check compile-time)\n";
}

int main(int argc, char** argv) {
    int width = 128;
    int height = 128;
    int steps = 100;
    int warmup = 5;
    unsigned long long seed = std::random_device{}();
    int use_per_thread_accum_flag = 0;
#if defined(USE_PER_THREAD_ACCUM)
    use_per_thread_accum_flag = 1;
#endif

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--width" && i + 1 < argc) {
            width = std::stoi(argv[++i]);
        } else if (arg == "--height" && i + 1 < argc) {
            height = std::stoi(argv[++i]);
        } else if (arg == "--steps" && i + 1 < argc) {
            steps = std::stoi(argv[++i]);
        } else if (arg == "--warmup" && i + 1 < argc) {
            warmup = std::stoi(argv[++i]);
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = std::stoull(argv[++i]);
        } else if (arg == "--use_per_thread_accum" && i + 1 < argc) {
            use_per_thread_accum_flag = std::stoi(argv[++i]);
        }
    }

    // Create UnitsCore with chosen precision
    UnitsCore core(width, height, 1.0, true);

    // Initialize with random values and targets
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            core.set_value(x, y, dist(rng));
        }
    }

    // Warmup
    for (int i = 0; i < warmup; ++i) {
        core.step();
    }

    // Benchmark
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < steps; ++i) {
        core.step();
    }
    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    double time_s = elapsed.count();
    double steps_per_s = steps / time_s;

    // Output JSON line
    std::cout << "{"
              << "\"width\":" << width << ","
              << "\"height\":" << height << ","
              << "\"steps\":" << steps << ","
              << "\"time_s\":" << time_s << ","
              << "\"steps_per_s\":" << steps_per_s << ","
              << "\"use_per_thread_accum\":" << (use_per_thread_accum_flag ? "true" : "false")
              << "}" << std::endl;

    return 0;
}
