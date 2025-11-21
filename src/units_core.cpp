#include "units_core.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

UnitsCore::UnitsCore(int width, int height, real_t max_value, bool torus)
    : m_width(width),
      m_height(height),
      m_max_value(max_value)
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
    // Pre-allocate per-thread accumulator buffer
    // We allocate a single flat buffer of size (num_threads * N)
    // Each thread gets a slice [thread_id * N, (thread_id + 1) * N)
    int num_threads = omp_get_max_threads();
    m_per_thread_accum.assign(static_cast<std::size_t>(num_threads) * N, 0.0);
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

void UnitsCore::set_value(int x, int y, real_t v)
{
    set_value_index(static_cast<std::size_t>(y) * m_width + x, v);
}

void UnitsCore::set_value_index(std::size_t idx, real_t v)
{
    if (idx >= m_values.size()) return;
    m_values[idx] = v;
}

real_t UnitsCore::value_at(int x, int y) const
{
    return value_at_index(static_cast<std::size_t>(y) * m_width + x);
}

real_t UnitsCore::value_at_index(std::size_t idx) const
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
        real_t v = m_values[i] + m_delta_steps[i] + m_deltas[i];
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
    // ============================================================================
    // Per-thread accumulator push algorithm (source-centric)
    // ============================================================================
    // Tradeoff: Avoids atomic operations by using per-thread local buffers.
    // Each thread accumulates contributions to a private buffer, then we merge
    // all per-thread buffers into m_delta_steps in a second parallel pass.
    //
    // Heuristic: This path is beneficial for large grids (1024x1024+) with many
    // threads (8+) where atomic contention becomes a bottleneck. For smaller grids
    // or fewer threads, the destination-centric path may be faster due to better
    // cache locality and less memory bandwidth usage.
    //
    // Memory: Allocates (num_threads * N) buffer. For 1024x1024 grid with 16 threads
    // and float, this is ~64MB. Pre-allocated in constructor to avoid per-step overhead.
    // ============================================================================

    const int num_threads = omp_get_max_threads();

    // Phase 1: Each thread accumulates into its own slice of m_per_thread_accum
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        real_t* thread_accum = &m_per_thread_accum[tid * N];
        
        // Zero out this thread's accumulator slice
        for (std::size_t i = 0; i < N; ++i) {
            thread_accum[i] = 0.0;
        }
        
        // Source-centric: each thread processes a subset of source cells
        #pragma omp for schedule(static) nowait
        for (std::size_t i = 0; i < N; ++i) {
            const int start = m_neighbor_index_start[i];
            const int end = m_neighbor_index_start[i + 1];
            const int degree = end - start;
            if (degree == 0) continue;
            
            real_t delta = m_deltas[i];
            real_t contrib = -delta / static_cast<real_t>(degree);
            
            // Accumulate to neighbors in this thread's local buffer
            for (int ni = start; ni < end; ++ni) {
                std::size_t nb = static_cast<std::size_t>(m_neighbors[ni]);
                thread_accum[nb] += contrib;
            }
        }
    }

    // Phase 2: Merge all per-thread accumulators into m_delta_steps
    // Each output cell is written by exactly one thread, so no atomics needed
    #pragma omp parallel for schedule(static)
    for (std::size_t i = 0; i < N; ++i) {
        real_t sum = 0.0;
        for (int tid = 0; tid < num_threads; ++tid) {
            sum += m_per_thread_accum[tid * N + i];
        }
        m_delta_steps[i] += sum;
    }

#else
    // ============================================================================
    // Destination-centric push algorithm with atomic accumulation
    // ============================================================================
    // Tradeoff: Uses atomic operations for thread-safe accumulation. Simple and
    // memory-efficient (only one temporary buffer of size N). Good cache locality
    // since we write to destinations that might be cached by other threads.
    //
    // Heuristic: Preferred for smaller grids (<512x512) or when OpenMP is not
    // available or when USE_PER_THREAD_ACCUM is not enabled. Atomic overhead is
    // acceptable when the number of concurrent writes to the same location is low.
    // ============================================================================

    // To enable safe parallelization we will accumulate contributions into a temporary buffer
    // then apply them to m_delta_steps. This avoids simultaneous writes to the same slot.
    std::vector<real_t> accum(N, 0.0);

#ifdef _OPENMP
    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (std::size_t i = 0; i < N; ++i) {
            const int start = m_neighbor_index_start[i];
            const int end = m_neighbor_index_start[i + 1];
            const int degree = end - start;
            if (degree == 0) continue;
            real_t delta = m_deltas[i];
            real_t contrib = -delta / static_cast<real_t>(degree); // amount to add to each neighbor
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
        real_t delta = m_deltas[i];
        real_t contrib = -delta / static_cast<real_t>(degree);
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