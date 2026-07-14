#include <benchmark/benchmark.h>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_set>

static void BM_StandardStringCopy(benchmark::State& state) {
    size_t strings_count = state.range(0);
    // Simulating heavy incoming keys
    std::vector<std::string> raw_source;
    for(size_t i=0; i<strings_count; ++i) {
        raw_source.push_back("HighlyVerboseAndExtremelyLongStringKeyPrefix_" + std::to_string(i));
    }

    for (auto _ : state) {
        std::vector<std::string> consumer;
        consumer.reserve(strings_count);
        for(const auto& str : raw_source) {
            consumer.push_back(str); // Force deep string copies/allocations
        }
        benchmark::DoNotOptimize(consumer.data());
    }
}
BENCHMARK(BM_StandardStringCopy)->Range(100, 2000);

static void BM_StringInterningView(benchmark::State& state) {
    size_t strings_count = state.range(0);
    std::unordered_set<std::string> global_intern_pool;
    std::vector<std::string_view> source_views;

    for(size_t i=0; i<strings_count; ++i) {
        auto [it, inserted] = global_intern_pool.insert("HighlyVerboseAndExtremelyLongStringKeyPrefix_" + std::to_string(i));
        source_views.push_back(*it);
    }

    for (auto _ : state) {
        std::vector<std::string_view> consumer;
        consumer.reserve(strings_count);
        for(const auto& view : source_views) {
            consumer.push_back(view); // Trivial, shallow 16-byte pointer copy
        }
        benchmark::DoNotOptimize(consumer.data());
    }
}
BENCHMARK(BM_StringInterningView)->Range(100, 2000);
