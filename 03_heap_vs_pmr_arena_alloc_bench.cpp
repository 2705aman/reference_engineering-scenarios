#include <benchmark/benchmark.h>
#include <vector>
#include <memory_resource>
#include <list>

static void BM_StandardHeapAllocation(benchmark::State& state) {
    size_t iterations = state.range(0);
    for (auto _ : state) {
        std::list<int> container;
        for (size_t i = 0; i < iterations; ++i) {
            container.push_back(static_cast<int>(i));
        }
        benchmark::DoNotOptimize(container);
    } // Forces continuous allocation/deallocation on the standard heap
}
BENCHMARK(BM_StandardHeapAllocation)->Range(1000, 50000);

static void BM_PmrArenaAllocation(benchmark::State& state) {
    size_t iterations = state.range(0);
    // Over-allocate a continuous pool of memory on the stack beforehand
    size_t required_bytes = iterations * 32; // Rough baseline per list node overhead
    auto underlying_buffer = std::make_unique<std::byte[]>(required_bytes);

    for (auto _ : state) {
        std::pmr::monotonic_buffer_resource pool(underlying_buffer.get(), required_bytes);
        std::pmr::list<int> container(&pool);
        
        for (size_t i = 0; i < iterations; ++i) {
            container.push_back(static_cast<int>(i));
        }
        benchmark::DoNotOptimize(container);
    } // Instantaneous batch cleanup without standard free calls
}
BENCHMARK(BM_PmrArenaAllocation)->Range(1000, 50000);
