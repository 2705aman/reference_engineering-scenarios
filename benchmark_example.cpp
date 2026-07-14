#include <benchmark/benchmark.h>
#include <vector>
#include <numeric>

// 1. Function to benchmark
static void BM_VectorAccumulate(benchmark::State& state) {
    // Setup code runs here (not timed)
    size_t elements = state.range(0);
    std::vector<int> data(elements, 42);

    // Timing loop
    for (auto _ : state) {
        long long sum = std::accumulate(data.begin(), data.end(), 0LL);
        
        // Prevent compiler from optimization deletion
        benchmark::DoNotOptimize(sum);
    }
    
    // Track throughput metadata
    state.SetItemsProcessed(state.iterations() * elements);
}

// 2. Register benchmark with input ranges (e.g., 1024, 8192, 65536)
BENCHMARK(BM_VectorAccumulate)->RangeMultiplier(8)->Range(1024, 65536);

// 3. Generate main entry point automatically
BENCHMARK_MAIN();
