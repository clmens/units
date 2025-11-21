#include "units_core.h"
#include <iostream>
#include <chrono>
#include <random>
#include <string>
#include <cstring>

// Simple command-line argument parser
struct BenchConfig {
    int width = 128;
    int height = 128;
    int steps = 100;
    int warmup = 5;
    unsigned long long seed = 12345;
    bool use_per_thread_accum = false;
};

BenchConfig parse_args(int argc, char** argv) {
    BenchConfig cfg;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--width" && i + 1 < argc) {
            cfg.width = std::stoi(argv[++i]);
        } else if (arg == "--height" && i + 1 < argc) {
            cfg.height = std::stoi(argv[++i]);
        } else if (arg == "--steps" && i + 1 < argc) {
            cfg.steps = std::stoi(argv[++i]);
        } else if (arg == "--warmup" && i + 1 < argc) {
            cfg.warmup = std::stoi(argv[++i]);
        } else if (arg == "--seed" && i + 1 < argc) {
            cfg.seed = std::stoull(argv[++i]);
        } else if (arg == "--use_per_thread_accum" && i + 1 < argc) {
            std::string val = argv[++i];
            cfg.use_per_thread_accum = (val == "1" || val == "true" || val == "True");
        }
    }
    return cfg;
}

int main(int argc, char** argv) {
    BenchConfig cfg = parse_args(argc, argv);

    // Create UnitsCore with random initial values
    UnitsCore core(cfg.width, cfg.height);

    std::mt19937_64 rng(cfg.seed);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    for (std::size_t i = 0; i < core.size(); ++i) {
        core.set_value_index(i, static_cast<value_t>(dist(rng)));
    }

    // Warmup
    for (int w = 0; w < cfg.warmup; ++w) {
        core.step();
    }

    // Timed benchmark
    auto start = std::chrono::steady_clock::now();
    for (int s = 0; s < cfg.steps; ++s) {
        core.step();
    }
    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    double time_s = elapsed.count();
    double steps_per_s = cfg.steps / time_s;

    // Output JSON line
    std::cout << "{"
              << "\"width\":" << cfg.width << ","
              << "\"height\":" << cfg.height << ","
              << "\"steps\":" << cfg.steps << ","
              << "\"time_s\":" << time_s << ","
              << "\"steps_per_s\":" << steps_per_s << ","
              << "\"use_per_thread_accum\":";
#if defined(USE_PER_THREAD_ACCUM)
    std::cout << "true";
#else
    std::cout << "false";
#endif
    std::cout << "}" << std::endl;

    return 0;
}
