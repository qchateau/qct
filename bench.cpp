#include <forward_list>
#include <random>
#include <set>

#include <benchmark/benchmark.h>

#include "avl.h"

using T = uint64_t;

class TNode : public Node<TNode> {
public:
    TNode(T x) : x_(x) {}

    friend auto operator<=>(TNode const& lhs, TNode const& rhs)
    {
        return lhs.x_ <=> rhs.x_;
    }

private:
    T x_;
};

constexpr auto find_init_size = 1000000;

static std::size_t seed()
{
    static auto seed = std::random_device{}();
    return seed;
}

static void BM_set_insert(benchmark::State& state)
{
    std::mt19937 gen(seed());
    std::uniform_int_distribution<T> distrib(0);

    std::multiset<T> tree;

    for (auto _ : state) {
        auto x = distrib(gen);
        benchmark::DoNotOptimize(tree.insert(x));
    }
}
BENCHMARK(BM_set_insert);

static void BM_avl_insert(benchmark::State& state)
{
    std::mt19937 gen(seed());
    std::uniform_int_distribution<T> distrib(0);

    AVLTree<TNode> tree;
    std::forward_list<TNode> nodes;

    for (auto _ : state) {
        auto x = distrib(gen);
        nodes.push_front(TNode{x});
        benchmark::DoNotOptimize(tree.insert(nodes.front()));
    }
}
BENCHMARK(BM_avl_insert);

static void BM_set_lower_bound(benchmark::State& state)
{
    std::mt19937 gen(seed());
    std::uniform_int_distribution<T> distrib(0);

    std::multiset<T> tree;

    for (std::size_t i = 0; i < find_init_size; ++i) {
        auto x = distrib(gen);
        tree.insert(x);
    }

    for (auto _ : state) {
        auto x = distrib(gen);
        benchmark::DoNotOptimize(tree.lower_bound(x));
    }
}
BENCHMARK(BM_set_lower_bound);

static void BM_avl_lower_bound(benchmark::State& state)
{
    std::mt19937 gen(seed());
    std::uniform_int_distribution<T> distrib(0);

    AVLTree<TNode> tree;
    std::forward_list<TNode> nodes;

    for (std::size_t i = 0; i < find_init_size; ++i) {
        auto x = distrib(gen);
        nodes.push_front(TNode{x});
        tree.insert(nodes.front());
    }

    for (auto _ : state) {
        auto x = distrib(gen);
        benchmark::DoNotOptimize(tree.lower_bound(x));
    }
}
BENCHMARK(BM_avl_lower_bound);

static void BM_set_lower_bound_distance(benchmark::State& state)
{
    std::mt19937 gen(seed());
    std::uniform_int_distribution<T> distrib(0);

    std::multiset<T> tree;

    for (std::size_t i = 0; i < find_init_size; ++i) {
        auto x = distrib(gen);
        tree.insert(x);
    }

    for (auto _ : state) {
        auto x = distrib(gen);
        benchmark::DoNotOptimize(distance(tree.begin(), tree.lower_bound(x)));
    }
}
BENCHMARK(BM_set_lower_bound_distance);

static void BM_avl_lower_bound_distance(benchmark::State& state)
{
    std::mt19937 gen(seed());
    std::uniform_int_distribution<T> distrib(0);

    AVLTree<TNode> tree;
    std::forward_list<TNode> nodes;

    for (std::size_t i = 0; i < find_init_size; ++i) {
        auto x = distrib(gen);
        nodes.push_front(TNode{x});
        tree.insert(nodes.front());
    }

    for (auto _ : state) {
        auto x = distrib(gen);
        benchmark::DoNotOptimize(
            AVLTree<TNode>::distance_from_begin(tree.lower_bound(x)));
    }
}
BENCHMARK(BM_avl_lower_bound_distance);

BENCHMARK_MAIN();
