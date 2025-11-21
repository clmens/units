#ifndef UNITS_CORE_H
#define UNITS_CORE_H

#include <vector>
#include <cstddef>

// Lightweight, cache-friendly core for Units simulation optimized for large grids.
// Stores values in flat arrays and neighbor indices as integer lists (torus wiring by default).
// Provides a simple two-phase step: update() then push(), and a convenience step() that runs both.

class UnitsCore {
public:
    UnitsCore(int width, int height, double max_value = 1.0, bool torus = true);

    int width() const { return m_width; }
    int height() const { return m_height; }
    std::size_t size() const { return m_values.size(); }

    void set_value(int x, int y, double v);
    void set_value_index(std::size_t idx, double v);
    double value_at(int x, int y) const;
    double value_at_index(std::size_t idx) const;

    // Simulation steps
    void update(); // integrate values, compute deltas
    void push();   // distribute deltas to neighbors (writes into delta_steps)
    void step() { update(); push(); }

    // Access raw buffers for visualization
    const std::vector<double>& values() const { return m_values; }

private:
    void build_neighbors(bool torus);

    int m_width;
    int m_height;
    double m_max_value;

    std::vector<double> m_values;
    std::vector<double> m_targets;
    std::vector<double> m_deltas;
    std::vector<double> m_delta_steps;

    // flattened neighbor indices: for each cell, store contiguous block of neighbor indices
    std::vector<int> m_neighbor_index_start; // start offset into m_neighbors per cell
    std::vector<int> m_neighbors; // concatenated neighbor lists

#if defined(USE_PER_THREAD_ACCUM) && defined(_OPENMP)
    // Per-thread accumulator buffer for lock-free push (compile-time option)
    std::vector<double> m_thread_accum;
#endif
};

#endif // UNITS_CORE_H
