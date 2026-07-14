#include <benchmark/benchmark.h>
#include <vector>
#include <set>
#include <algorithm>
#include <random>

class ContainerBenchmark : public benchmark::Fixture {
protected:
    std::vector<int> random_keys;
    std::set<int> node_set;
    std::vector<int> flat_vector;

    void SetUp(const ::benchmark::State& state) override {
        size_t elements = state.range(0);
        std::mt19937 gen(1337);
        std::uniform_int_distribution<int> dist(0, 1000000);

        node_set.clear();
        flat_vector.clear();
        random_keys.resize(1000);

        for(size_t i=0; i<elements; ++i) {
            int val = dist(gen);
            node_set.insert(val);
            flat_vector.push_back(val);
        }
        std::sort(flat_vector.begin(), flat_vector.end());

        for(size_t i=0; i<1000; ++i) {
            random_keys[i] = dist(gen);
        }
    }
};

BENCHMARK_F(ContainerBenchmark, NodeSetLookup)(benchmark::State& state) {
    for (auto _ : state) {
        long long hits = 0;
        for (int key : random_keys) {
            if (node_set.find(key) != node_set.end()) {
                hits++;
            }
        }
        benchmark::DoNotOptimize(hits);
    }
}

BENCHMARK_F(ContainerBenchmark, FlatVectorLookup)(benchmark::State& state) {
    for (auto _ : state) {
        long long hits = 0;
        for (int key : random_keys) {
            if (std::binary_search(flat_vector.begin(), flat_vector.end(), key)) {
                hits++;
            }
        }
        benchmark::DoNotOptimize(hits);
    }
}

BENCHMARK_REGISTER_F(ContainerBenchmark, NodeSetLookup)->Range(1000, 10000);
BENCHMARK_REGISTER_F(ContainerBenchmark, FlatVectorLookup)->Range(1000, 10000);
