#ifndef UNITS_CORE_H
#define UNITS_CORE_H

#include <vector>
#include <cstddef>

// Lightweight, cache-friendly core for Units simulation optimized for large grids.
// Stores values in flat arrays and neighbor indices as integer lists (torus wiring by default).
// Provides a simple two-phase step: update() then push(), and a convenience step() that runs both.

// Precision selection: USE_FLOAT for float, otherwise double
#ifdef USE_FLOAT
    using value_t = float;
#else
    using value_t = double;
#endif

class UnitsCore {
public:
    UnitsCore(int width, int height, value_t max_value = 1.0, bool torus = true);

    int width() const { return m_width; }
    int height() const { return m_height; }
    std::size_t size() const { return m_values.size(); }

    void set_value(int x, int y, value_t v);
    void set_value_index(std::size_t idx, value_t v);
    value_t value_at(int x, int y) const;
    value_t value_at_index(std::size_t idx) const;

    // Simulation steps
    void update(); // integrate values, compute deltas
    void push();   // distribute deltas to neighbors (writes into delta_steps)
    void step() { update(); push(); }

    // Access raw buffers for visualization
    const std::vector<value_t>& values() const { return m_values; }

private:
    void build_neighbors(bool torus);

    int m_width;
    int m_height;
    value_t m_max_value;

    std::vector<value_t> m_values;
    std::vector<value_t> m_targets;
    std::vector<value_t> m_deltas;
    std::vector<value_t> m_delta_steps;

    // flattened neighbor indices: for each cell, store contiguous block of neighbor indices
    std::vector<int> m_neighbor_index_start; // start offset into m_neighbors per cell
    std::vector<int> m_neighbors; // concatenated neighbor lists

#if defined(USE_PER_THREAD_ACCUM) && defined(_OPENMP)
    // Per-thread accumulator buffer to avoid atomics in push()
    // Layout: thread_accum[thread_id * N + cell_index]
    std::vector<value_t> m_thread_accum;
    int m_num_threads;
#endif
};

#endif // UNITS_CORE_H
