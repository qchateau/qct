#include <chrono>
#include <list>
#include <random>
#include <variant>

#include <benchmark/benchmark.h>
#include <boost/intrusive/avl_set.hpp>

#include "qct.h"

template <typename T>
class comparable_node : public qct::node<> {
public:
    using value_type = T;

    explicit comparable_node(T x) : x_(x) {}

    friend auto operator<=>(
        comparable_node const& lhs,
        comparable_node const& rhs) = default;

private:
    T x_;
};

template <typename T>
class boost_avl_node : public boost::intrusive::avl_set_base_hook<> {
public:
    using value_type = T;

    explicit boost_avl_node(T x) : x_(x) {}

    friend auto operator<=>(boost_avl_node const& lhs, boost_avl_node const& rhs)
    {
        return lhs.x_ <=> rhs.x_;
    }

private:
    T x_;
};

constexpr auto a = sizeof(comparable_node<int64_t>);
constexpr auto b = sizeof(boost_avl_node<int64_t>);
constexpr auto init_size = 100000;

static std::size_t seed()
{
    static auto seed = std::random_device{}();
    return seed;
}

template <typename Data>
static auto init_rng()
{
    std::mt19937 gen(seed());
    return [gen = std::mt19937{seed()},
            distrib = std::uniform_int_distribution<Data>(
                std::numeric_limits<Data>::min(),
                std::numeric_limits<Data>::max())]() mutable {
        return distrib(gen);
    };
}

template <template <typename...> typename TreeT, typename Node>
static auto init_tree()
{
    using Tree = TreeT<Node>;
    auto distrib = init_rng<typename Node::value_type>();

    std::vector<std::unique_ptr<Node>> nodes;
    Tree tree;

    for (std::size_t i = 0; i < init_size; ++i) {
        auto x = distrib();
        nodes.push_back(std::make_unique<Node>(x));
        tree.insert(*nodes.back());
    }

    return std::tuple{std::move(tree), std::move(nodes)};
}

class iteration_timer {
public:
    explicit iteration_timer(benchmark::State& state)
        : state_{state}, start_{std::chrono::steady_clock::now()}
    {
    }

    ~iteration_timer()
    {
        auto const duration = std::chrono::steady_clock::now() - start_;
        state_.SetIterationTime(
            std::chrono::duration_cast<std::chrono::duration<double>>(duration)
                .count());
    }

private:
    benchmark::State& state_;
    std::chrono::steady_clock::time_point start_;
};

template <template <typename...> typename TreeT, typename Node>
static void BM_insert(benchmark::State& state)
{
    auto [tree, nodes] = init_tree<TreeT, Node>();
    auto distrib = init_rng<typename Node::value_type>();

    for (auto _ : state) {
        auto x = distrib();
        auto node = std::make_unique<Node>(x);
        auto it = tree.end();

        {
            iteration_timer _(state);
            it = tree.insert(*node);
            benchmark::DoNotOptimize(it);
        }

        tree.erase(it);
    }
}

template <template <typename...> typename TreeT, typename Node>
static void BM_erase(benchmark::State& state)
{
    auto [tree, nodes] = init_tree<TreeT, Node>();
    auto distrib = init_rng<typename Node::value_type>();

    for (auto _ : state) {
        auto it = tree.end();
        while (it == tree.end()) {
            it = tree.lower_bound(Node{distrib()});
        }

        {
            iteration_timer _(state);
            benchmark::DoNotOptimize(tree.erase(it));
        }

        tree.insert(*it);
    }
}

template <template <typename...> typename TreeT, typename Node>
static void BM_find(benchmark::State& state)
{
    auto [tree, nodes] = init_tree<TreeT, Node>();
    auto distrib = init_rng<typename Node::value_type>();

    for (auto _ : state) {
        auto const& x = *nodes[std::abs(distrib()) % nodes.size()];
        benchmark::DoNotOptimize(tree.find(x));
    }
}

template <template <typename...> typename TreeT, typename Node>
static void BM_lower_bound(benchmark::State& state)
{
    auto [tree, nodes] = init_tree<TreeT, Node>();
    auto distrib = init_rng<typename Node::value_type>();

    for (auto _ : state) {
        auto x = distrib();
        benchmark::DoNotOptimize(tree.lower_bound(Node{x}));
    }
}

template <template <typename...> typename TreeT, typename Node>
static void BM_equal_range(benchmark::State& state)
{
    auto [tree, nodes] = init_tree<TreeT, Node>();
    auto distrib = init_rng<typename Node::value_type>();

    for (auto _ : state) {
        auto const& x = *nodes[std::abs(distrib()) % nodes.size()];
        benchmark::DoNotOptimize(tree.equal_range(x));
    }
}

template <template <typename...> typename TreeT, typename Node>
static void BM_distance(benchmark::State& state)
{
    auto [tree, nodes] = init_tree<TreeT, Node>();
    auto distrib = init_rng<typename Node::value_type>();

    for (auto _ : state) {
        auto a = distrib();
        auto b = distrib();
        if (b < a) {
            std::swap(a, b);
        }
        auto ita = tree.lower_bound(Node{a});
        auto itb = tree.lower_bound(Node{b});

        {
            iteration_timer _(state);
            using std::distance;
            benchmark::DoNotOptimize(distance(ita, itb));
        }
    }
}

template <template <typename...> typename TreeT, typename Node>
static void BM_iter(benchmark::State& state)
{
    auto [tree, nodes] = init_tree<TreeT, Node>();

    for (auto _ : state) {
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            benchmark::DoNotOptimize(it);
        }
    }
}

template <template <typename...> typename TreeT, typename Node>
static void BM_reverse_iter(benchmark::State& state)
{
    auto [tree, nodes] = init_tree<TreeT, Node>();

    for (auto _ : state) {
        for (auto it = tree.end(); it != tree.begin(); --it) {
            benchmark::DoNotOptimize(it);
        }
    }
}

static void BM_qct_insert(benchmark::State& state)
{
    BM_insert<qct::tree, comparable_node<int64_t>>(state);
}
BENCHMARK(BM_qct_insert);

static void BM_boost_avl_insert(benchmark::State& state)
{
    BM_insert<boost::intrusive::avl_multiset, boost_avl_node<int64_t>>(state);
}
BENCHMARK(BM_boost_avl_insert);

static void BM_qct_erase(benchmark::State& state)
{
    BM_erase<qct::tree, comparable_node<int64_t>>(state);
}
BENCHMARK(BM_qct_erase);

static void BM_boost_avl_erase(benchmark::State& state)
{
    BM_erase<boost::intrusive::avl_multiset, boost_avl_node<int64_t>>(state);
}
BENCHMARK(BM_boost_avl_erase);

static void BM_qct_find(benchmark::State& state)
{
    BM_find<qct::tree, comparable_node<int64_t>>(state);
}
BENCHMARK(BM_qct_find);

static void BM_boost_avl_find(benchmark::State& state)
{
    BM_find<boost::intrusive::avl_multiset, boost_avl_node<int64_t>>(state);
}
BENCHMARK(BM_boost_avl_find);

static void BM_qct_lower_bound(benchmark::State& state)
{
    BM_lower_bound<qct::tree, comparable_node<int64_t>>(state);
}
BENCHMARK(BM_qct_lower_bound);

static void BM_boost_avl_lower_bound(benchmark::State& state)
{
    BM_lower_bound<boost::intrusive::avl_multiset, boost_avl_node<int64_t>>(state);
}
BENCHMARK(BM_boost_avl_lower_bound);

static void BM_qct_equal_range(benchmark::State& state)
{
    BM_equal_range<qct::tree, comparable_node<int64_t>>(state);
}
BENCHMARK(BM_qct_equal_range);

static void BM_boost_avl_equal_range(benchmark::State& state)
{
    BM_equal_range<boost::intrusive::avl_multiset, boost_avl_node<int64_t>>(state);
}
BENCHMARK(BM_boost_avl_equal_range);

static void BM_qct_distance(benchmark::State& state)
{
    BM_distance<qct::tree, comparable_node<int64_t>>(state);
}
BENCHMARK(BM_qct_distance);

static void BM_boost_avl_distance(benchmark::State& state)
{
    BM_distance<boost::intrusive::avl_multiset, boost_avl_node<int64_t>>(state);
}
BENCHMARK(BM_boost_avl_distance);

static void BM_qct_iter(benchmark::State& state)
{
    BM_iter<qct::tree, comparable_node<int64_t>>(state);
}
BENCHMARK(BM_qct_iter);

static void BM_boost_avl_iter(benchmark::State& state)
{
    BM_iter<boost::intrusive::avl_multiset, boost_avl_node<int64_t>>(state);
}
BENCHMARK(BM_boost_avl_iter);

static void BM_qct_reverse_iter(benchmark::State& state)
{
    BM_reverse_iter<qct::tree, comparable_node<int64_t>>(state);
}
BENCHMARK(BM_qct_reverse_iter);

static void BM_boost_avl_reverse_iter(benchmark::State& state)
{
    BM_reverse_iter<boost::intrusive::avl_multiset, boost_avl_node<int64_t>>(
        state);
}
BENCHMARK(BM_boost_avl_reverse_iter);

BENCHMARK_MAIN();
