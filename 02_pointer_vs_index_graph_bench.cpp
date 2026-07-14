#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <memory>

// Classical Pointer-based Node (8 bytes pointer + 8 bytes payload = 16 bytes minimum per edge)
struct PointerNode {
    int payload;
    std::vector<PointerNode*> neighbors;
};

// Compact Index-based Node (4 bytes index + 8 bytes payload = 12 bytes per edge, contiguous allocation)
struct IndexNode {
    int payload;
    uint32_t edge_start;
    uint32_t edge_count;
};

class GraphBenchmark : public benchmark::Fixture {
protected:
    std::vector<std::unique_ptr<PointerNode>> pointer_graph;
    
    std::vector<IndexNode> index_nodes;
    std::vector<uint32_t> index_edges; // Flattened layout

    void SetUp(const ::benchmark::State& state) override {
        size_t num_nodes = state.range(0);
        size_t edges_per_node = 4;

        // Initialize Pointer Graph (Node allocations are scattered across heap)
        pointer_graph.clear();
        for (size_t i = 0; i < num_nodes; ++i) {
            auto node = std::make_unique<PointerNode>();
            node->payload = static_cast<int>(i);
            pointer_graph.push_back(std::move(node));
        }
        for (size_t i = 0; i < num_nodes; ++i) {
            for (size_t j = 0; j < edges_per_node; ++j) {
                pointer_graph[i]->neighbors.push_back(pointer_graph[(i + j + 1) % num_nodes].get());
            }
        }

        // Initialize Index Graph (Contiguous memory pools)
        index_nodes.resize(num_nodes);
        index_edges.resize(num_nodes * edges_per_node);
        uint32_t edge_idx = 0;
        for (size_t i = 0; i < num_nodes; ++i) {
            index_nodes[i] = { static_cast<int>(i), edge_idx, static_cast<uint32_t>(edges_per_node) };
            for (size_t j = 0; j < edges_per_node; ++j) {
                index_edges[edge_idx++] = static_cast<uint32_t>((i + j + 1) % num_nodes);
            }
        }
    }
};

BENCHMARK_F(GraphBenchmark, PointerTraversal)(benchmark::State& state) {
    for (auto _ : state) {
        long long total = 0;
        for (const auto& node : pointer_graph) {
            for (const auto* neighbor : node->neighbors) {
                total += neighbor->payload;
            }
        }
        benchmark::DoNotOptimize(total);
    }
}

BENCHMARK_F(GraphBenchmark, IndexTraversal)(benchmark::State& state) {
    for (auto _ : state) {
        long long total = 0;
        for (const auto& node : index_nodes) {
            uint32_t start = node.edge_start;
            uint32_t end = start + node.edge_count;
            for (uint32_t i = start; i < end; ++i) {
                uint32_t target_idx = index_edges[i];
                total += index_nodes[target_idx].payload;
            }
        }
        benchmark::DoNotOptimize(total);
    }
}

BENCHMARK_REGISTER_F(GraphBenchmark, PointerTraversal)->Range(1000, 100000);
BENCHMARK_REGISTER_F(GraphBenchmark, IndexTraversal)->Range(1000, 100000);
