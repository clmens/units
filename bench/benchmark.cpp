#include "units_core.h"
#include <iostream>
#include <random>
#include <chrono>
#include <string>
#include <cstring>

#ifdef _OPENMP
#include <omp.h>
#endif

// Simple CLI argument parser
struct BenchConfig {
    int width = 128;
    int height = 128;
    int steps = 500;
    int warmup = 5;
    unsigned int seed = 12345;
};

BenchConfig parse_args(int argc, char** argv) {
    BenchConfig cfg;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--width" || arg == "-w") && i + 1 < argc) {
            cfg.width = std::stoi(argv[++i]);
        } else if ((arg == "--height" || arg == "-h") && i + 1 < argc) {
            cfg.height = std::stoi(argv[++i]);
        } else if ((arg == "--steps" || arg == "-s") && i + 1 < argc) {
            cfg.steps = std::stoi(argv[++i]);
        } else if (arg == "--warmup" && i + 1 < argc) {
            cfg.warmup = std::stoi(argv[++i]);
        } else if (arg == "--seed" && i + 1 < argc) {
            cfg.seed = static_cast<unsigned int>(std::stoul(argv[++i]));
        } else if (arg == "--help") {
            std::cout << "Usage: bench_units [options]\n"
                      << "  --width <W>      Grid width (default: 128)\n"
                      << "  --height <H>     Grid height (default: 128)\n"
                      << "  --steps <S>      Number of simulation steps (default: 500)\n"
                      << "  --warmup <N>     Number of warmup steps (default: 5)\n"
                      << "  --seed <S>       Random seed (default: 12345)\n"
                      << "  --help           Show this help\n"
                      << "\n"
                      << "Build-time options (set via CMake):\n"
                      << "  USE_FLOAT        Use float instead of double\n"
                      << "  USE_OPENMP       Enable OpenMP parallelization\n"
                      << "  USE_PER_THREAD_ACCUM  Use per-thread accumulators\n";
            std::exit(0);
        }
    }
    return cfg;
}

int main(int argc, char** argv) {
    BenchConfig cfg = parse_args(argc, argv);

    // Validate inputs
    if (cfg.width <= 0 || cfg.height <= 0 || cfg.steps <= 0) {
        std::cerr << "Error: width, height, and steps must be positive\n";
        return 1;
    }

    const std::size_t N = static_cast<std::size_t>(cfg.width) * static_cast<std::size_t>(cfg.height);

    // Create UnitsCore with random initial values
    UnitsCore core(cfg.width, cfg.height, 1.0, true);

    // Initialize with random values
    std::mt19937 rng(cfg.seed);
    std::uniform_real_distribution<units_real> dist(-1.0, 1.0);
    for (std::size_t i = 0; i < N; ++i) {
        core.set_value_index(i, dist(rng));
    }

    // Warmup: run a few steps to ensure everything is loaded into cache
    for (int i = 0; i < cfg.warmup; ++i) {
        core.step();
    }

    // Benchmark: measure steady-state time
    auto start_time = std::chrono::steady_clock::now();
    
    for (int i = 0; i < cfg.steps; ++i) {
        core.step();
    }
    
    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    double time_s = elapsed.count();
    double steps_per_s = cfg.steps / time_s;

    // Determine number of threads
    int num_threads = 1;
#ifdef _OPENMP
    num_threads = omp_get_max_threads();
#endif

    // Determine precision
    const char* precision = 
#ifdef UNITS_USE_FLOAT
        "float";
#else
        "double";
#endif

    // Print results as single JSON line
    std::cout << "{\"width\": " << cfg.width
              << ", \"height\": " << cfg.height
              << ", \"steps\": " << cfg.steps
              << ", \"time_s\": " << time_s
              << ", \"steps_per_s\": " << steps_per_s
              << ", \"use_per_thread_accum\": "
#ifdef USE_PER_THREAD_ACCUM
              << "true"
#else
              << "false"
#endif
              << ", \"threads\": " << num_threads
              << ", \"precision\": \"" << precision << "\""
              << "}\n";

    return 0;
}
