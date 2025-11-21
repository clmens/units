#include "units_core.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

UnitsCore::UnitsCore(int width, int height, value_t max_value, bool torus)
    : m_width(width),
      m_height(height),
      m_max_value(max_value)
#if defined(USE_PER_THREAD_ACCUM) && defined(_OPENMP)
      , m_num_threads(1)
#endif
{
    if (width <= 0 || height <= 0) throw std::invalid_argument("width/height must be > 0");

    const std::size_t N = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    m_values.assign(N, 0.0);
    m_targets.assign(N, 0.0);
    m_deltas.assign(N, 0.0);
    m_delta_steps.assign(N, 0.0);

    m_neighbor_index_start.assign(N + 1, 0); // extra sentinel at end
    build_neighbors(torus);

#if defined(USE_PER_THREAD_ACCUM) && defined(_OPENMP)
    // Initialize per-thread accumulator buffer
    #pragma omp parallel
    {
        #pragma omp single
        {
            m_num_threads = omp_get_num_threads();
        }
    }
    m_thread_accum.assign(m_num_threads * N, 0.0);
#endif
}

void UnitsCore::build_neighbors(bool torus)
{
    const int W = m_width;
    const int H = m_height;
    const std::size_t N = static_cast<std::size_t>(W) * static_cast<std::size_t>(H);

    // First pass: count neighbors per cell (we use 8-neighbour stencil)
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            std::size_t idx = static_cast<std::size_t>(y) * W + x;
            int count = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = x + dx;
                    int ny = y + dy;
                    if (torus) {
                        nx = (nx + W) % W;
                        ny = (ny + H) % H;
                    } else {
                        if (nx < 0 || nx >= W || ny < 0 || ny >= H) continue;
                    }
                    ++count;
                }
            }
            m_neighbor_index_start[idx + 1] = m_neighbor_index_start[idx] + count;
        }
    }

    // allocate flattened neighbor vector
    m_neighbors.resize(m_neighbor_index_start[N]);

    // fill neighbors
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            std::size_t idx = static_cast<std::size_t>(y) * W + x;
            int write_pos = m_neighbor_index_start[idx];
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = x + dx;
                    int ny = y + dy;
                    if (torus) {
                        nx = (nx + W) % W;
                        ny = (ny + H) % H;
                    } else {
                        if (nx < 0 || nx >= W || ny < 0 || ny >= H) continue;
                    }
                    m_neighbors[write_pos++] = ny * W + nx;
                }
            }
            // assert write_pos == m_neighbor_index_start[idx+1];
        }
    }
}

void UnitsCore::set_value(int x, int y, value_t v)
{
    set_value_index(static_cast<std::size_t>(y) * m_width + x, v);
}

void UnitsCore::set_value_index(std::size_t idx, value_t v)
{
    if (idx >= m_values.size()) return;
    m_values[idx] = v;
}

value_t UnitsCore::value_at(int x, int y) const
{
    return value_at_index(static_cast<std::size_t>(y) * m_width + x);
}

value_t UnitsCore::value_at_index(std::size_t idx) const
{
    return m_values[idx];
}

void UnitsCore::update()
{
    const std::size_t N = m_values.size();

    // Integrate delta_step + delta into values, clamp, and compute new delta
    // Parallelizable: each index writes to its own slot
#ifdef _OPENMP
    #pragma omp parallel for schedule(static)
#endif
    for (std::size_t i = 0; i < N; ++i) {
        value_t v = m_values[i] + m_delta_steps[i] + m_deltas[i];
        if (v > m_max_value) v = m_max_value;
        else if (v < -m_max_value) v = -m_max_value;
        m_values[i] = v;
        m_deltas[i] = (m_targets[i] - v);
        m_delta_steps[i] = 0.0; // clear for next push phase
    }
}

void UnitsCore::push()
{
    const std::size_t N = m_values.size();

#if defined(USE_PER_THREAD_ACCUM) && defined(_OPENMP)
    // Per-thread accumulator approach: avoid atomics by using per-thread buffers
    // Fast-path heuristic: best for large N (>= 64*64) with many cores and low-degree graphs
    // For small N or single-threaded builds, prefer destination-centric approach below

    // Clear thread accumulator buffer using memset for performance
    std::fill(m_thread_accum.begin(), m_thread_accum.end(), 0.0);

    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        value_t* my_accum = &m_thread_accum[tid * N];

        #pragma omp for schedule(static)
        for (std::size_t i = 0; i < N; ++i) {
            const int start = m_neighbor_index_start[i];
            const int end = m_neighbor_index_start[i + 1];
            const int degree = end - start;
            if (degree == 0) continue;
            value_t delta = m_deltas[i];
            value_t contrib = -delta / static_cast<value_t>(degree);
            for (int ni = start; ni < end; ++ni) {
                std::size_t nb = static_cast<std::size_t>(m_neighbors[ni]);
                my_accum[nb] += contrib;
            }
        }
    }

    // Merge per-thread contributions into m_delta_steps (single-threaded)
    for (int t = 0; t < m_num_threads; ++t) {
        const value_t* thread_data = &m_thread_accum[t * N];
        for (std::size_t i = 0; i < N; ++i) {
            m_delta_steps[i] += thread_data[i];
        }
    }

#else
    // Destination-centric approach with atomics (or serial)
    // Preferred for small grids or when per-thread accum is not available
    std::vector<value_t> accum(N, 0.0);

#ifdef _OPENMP
    #pragma omp parallel
    {
        // thread-local accumulator to reduce contention (optional)
        std::vector<value_t> local_accum;
        local_accum.assign(0, 0.0); // will be resized on first use
        #pragma omp for schedule(static)
        for (std::size_t i = 0; i < N; ++i) {
            const int start = m_neighbor_index_start[i];
            const int end = m_neighbor_index_start[i + 1];
            const int degree = end - start;
            if (degree == 0) continue;
            value_t delta = m_deltas[i];
            value_t contrib = -delta / static_cast<value_t>(degree); // amount to add to each neighbor
            for (int ni = start; ni < end; ++ni) {
                std::size_t nb = static_cast<std::size_t>(m_neighbors[ni]);
                #pragma omp atomic
                accum[nb] += contrib;
            }
        }
    }
#else
    for (std::size_t i = 0; i < N; ++i) {
        const int start = m_neighbor_index_start[i];
        const int end = m_neighbor_index_start[i + 1];
        const int degree = end - start;
        if (degree == 0) continue;
        value_t delta = m_deltas[i];
        value_t contrib = -delta / static_cast<value_t>(degree);
        for (int ni = start; ni < end; ++ni) {
            std::size_t nb = static_cast<std::size_t>(m_neighbors[ni]);
            accum[nb] += contrib;
        }
    }
#endif

    // Apply accumulation to delta_steps
    for (std::size_t i = 0; i < N; ++i) {
        m_delta_steps[i] += accum[i];
    }
#endif
}