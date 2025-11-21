#ifndef UNITS_CORE_H
#define UNITS_CORE_H

#include <vector>
#include <cstddef>

// Lightweight, cache-friendly core for Units simulation optimized for large grids.
// Stores values in flat arrays and neighbor indices as integer lists (torus wiring by default).
// Provides a simple two-phase step: update() then push(), and a convenience step() that runs both.

// Allow compile-time choice of float vs double precision
// Define UNITS_USE_FLOAT to prefer float (smaller memory footprint)
#ifdef UNITS_USE_FLOAT
using units_real = float;
#else
using units_real = double;
#endif

class UnitsCore {
public:
    UnitsCore(int width, int height, units_real max_value = 1.0, bool torus = true);

    int width() const { return m_width; }
    int height() const { return m_height; }
    std::size_t size() const { return m_values.size(); }

    void set_value(int x, int y, units_real v);
    void set_value_index(std::size_t idx, units_real v);
    units_real value_at(int x, int y) const;
    units_real value_at_index(std::size_t idx) const;

    // Simulation steps
    void update(); // integrate values, compute deltas
    void push();   // distribute deltas to neighbors (writes into delta_steps)
    void step() { update(); push(); }

    // Access raw buffers for visualization
    const std::vector<units_real>& values() const { return m_values; }

private:
    void build_neighbors(bool torus);

    int m_width;
    int m_height;
    units_real m_max_value;

    std::vector<units_real> m_values;
    std::vector<units_real> m_targets;
    std::vector<units_real> m_deltas;
    std::vector<units_real> m_delta_steps;

    // flattened neighbor indices: for each cell, store contiguous block of neighbor indices
    std::vector<int> m_neighbor_index_start; // start offset into m_neighbors per cell
    std::vector<int> m_neighbors; // concatenated neighbor lists

#if defined(USE_PER_THREAD_ACCUM)
    // Per-thread accumulator buffer for push algorithm (allocated once, reused each step)
    // Type matches the chosen precision (units_real). Allocation and use should be
    // guarded by USE_PER_THREAD_ACCUM and OpenMP at the implementation sites.
    std::vector<units_real> m_per_thread_accum;
#endif
};

#endif // UNITS_CORE_H