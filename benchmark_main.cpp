#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <numeric>

// ============================================================================
// 1. Data Structure Definitions
// ============================================================================

// Array of Structures (AoS) Layout
struct ParticleAoS {
    float x, y, z;
    float vx, vy, vz;
    float mass;
    int id;
};

// Structure of Arrays (SoA) Layout
struct ParticleSoA {
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<float> mass;
    std::vector<int> id;

    void resize(size_t size) {
        x.resize(size); y.resize(size); z.resize(size);
        vx.resize(size); vy.resize(size); vz.resize(size);
        mass.resize(size);
        id.resize(size);
    }
};

// ============================================================================
// 2. Benchmark Fixture for Shared Setup
// ============================================================================
class LayoutBenchmark : public benchmark::Fixture {
protected:
    std::vector<ParticleAoS> aos_data;
    ParticleSoA soa_data;
    float dt = 0.01f;

    void SetUp(const ::benchmark::State& state) override {
        size_t num_elements = state.range(0);
        
        // Initialize AoS
        aos_data.resize(num_elements);
        std::mt19937 gen(42);
        std::uniform_real_distribution<float> dist(-100.0f, 100.0f);

        for (size_t i = 0; i < num_elements; ++i) {
            aos_data[i] = {
                dist(gen), dist(gen), dist(gen),
                dist(gen), dist(gen), dist(gen),
                dist(gen), static_cast<int>(i)
            };
        }

        // Initialize SoA
        soa_data.resize(num_elements);
        for (size_t i = 0; i < num_elements; ++i) {
            soa_data.x[i] = aos_data[i].x;
            soa_data.y[i] = aos_data[i].y;
            soa_data.z[i] = aos_data[i].z;
            soa_data.vx[i] = aos_data[i].vx;
            soa_data.vy[i] = aos_data[i].vy;
            soa_data.vz[i] = aos_data[i].vz;
            soa_data.mass[i] = aos_data[i].mass;
            soa_data.id[i] = aos_data[i].id;
        }
    }
};

// ============================================================================
// 3. Benchmark Implementations
// ============================================================================

// Benchmarking AoS: Iterating through an array of objects (poor cache line use for specific fields)
BENCHMARK_F(LayoutBenchmark, MeasureAoS)(benchmark::State& state) {
    for (auto _ : state) {
        for (auto& particle : aos_data) {
            particle.x += particle.vx * dt;
            particle.y += particle.vy * dt;
            particle.z += particle.vz * dt;
        }
        // Force the compiler not to optimize away the operations
        benchmark::DoNotOptimize(aos_data.data());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}

// Benchmarking SoA: Contiguous memory access (highly cache-friendly, auto-vectorizable)
BENCHMARK_F(LayoutBenchmark, MeasureSoA)(benchmark::State& state) {
    for (auto _ : state) {
        size_t size = state.range(0);
        float* alignment_x = soa_data.x.data();
        float* alignment_y = soa_data.y.data();
        float* alignment_z = soa_data.z.data();
        const float* alignment_vx = soa_data.vx.data();
        const float* alignment_vy = soa_data.vy.data();
        const float* alignment_vz = soa_data.vz.data();

        #pragma omp simd // Hint to compiler for vectorization
        for (size_t i = 0; i < size; ++i) {
            alignment_x[i] += alignment_vx[i] * dt;
            alignment_y[i] += alignment_vy[i] * dt;
            alignment_z[i] += alignment_vz[i] * dt;
        }
        benchmark::DoNotOptimize(soa_data.x.data());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}

// ============================================================================
// 4. Execution Arguments (Testing sizes from 1K to 1M elements)
// ============================================================================
BENCHMARK_REGISTER_F(LayoutBenchmark, MeasureAoS)
    ->RangeMultiplier(10)->Range(1000, 1000000);

BENCHMARK_REGISTER_F(LayoutBenchmark, MeasureSoA)
    ->RangeMultiplier(10)->Range(1000, 1000000);

BENCHMARK_MAIN();
