#ifndef UNITS_CORE_H
#define UNITS_CORE_H

#include <vector>
#include <cstddef>

// Conditional typedef for real_t based on USE_FLOAT macro
#ifdef USE_FLOAT
    using real_t = float;
#else
    using real_t = double;
#endif

// Lightweight, cache-friendly core for Units simulation optimized for large grids.
// Stores values in flat arrays and neighbor indices as integer lists (torus wiring by default).
// Provides a simple two-phase step: update() then push(), and a convenience step() that runs both.

class UnitsCore {
public:
    UnitsCore(int width, int height, real_t max_value = 1.0, bool torus = true);

    int width() const { return m_width; }
    int height() const { return m_height; }
    std::size_t size() const { return m_values.size(); }

    void set_value(int x, int y, real_t v);
    void set_value_index(std::size_t idx, real_t v);
    real_t value_at(int x, int y) const;
    real_t value_at_index(std::size_t idx) const;

    // Simulation steps
    void update(); // integrate values, compute deltas
    void push();   // distribute deltas to neighbors (writes into delta_steps)
    void step() { update(); push(); }

    // Access raw buffers for visualization
    const std::vector<real_t>& values() const { return m_values; }

private:
    void build_neighbors(bool torus);

    int m_width;
    int m_height;
    real_t m_max_value;

    std::vector<real_t> m_values;
    std::vector<real_t> m_targets;
    std::vector<real_t> m_deltas;
    std::vector<real_t> m_delta_steps;

    // flattened neighbor indices: for each cell, store contiguous block of neighbor indices
    std::vector<int> m_neighbor_index_start; // start offset into m_neighbors per cell
    std::vector<int> m_neighbors; // concatenated neighbor lists

#if defined(USE_PER_THREAD_ACCUM) && defined(_OPENMP)
    // Per-thread accumulator buffer for push algorithm (allocated once, reused each step)
    std::vector<real_t> m_per_thread_accum;
#endif
};

#endif // UNITS_CORE_H
