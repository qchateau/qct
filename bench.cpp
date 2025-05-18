#include <list>
#include <random>
#include <set>
#include <variant>

#include <benchmark/benchmark.h>
#include <boost/intrusive/avl_set.hpp>
#include <boost/intrusive/set.hpp>

#include "avl.h"

template <typename T>
class qct_node : public qct::avl_node<> {
public:
    using value_type = T;

    qct_node(T x) : x_(x) {}

    friend auto operator<=>(qct_node const& lhs, qct_node const& rhs)
    {
        return lhs.x_ <=> rhs.x_;
    }

private:
    T x_;
};

template <typename T>
class boost_rb_node : public boost::intrusive::set_base_hook<> {
public:
    using value_type = T;

    boost_rb_node(T x) : x_(x) {}

    friend auto operator<=>(boost_rb_node const& lhs, boost_rb_node const& rhs)
    {
        return lhs.x_ <=> rhs.x_;
    }

private:
    T x_;
};

template <typename T>
class boost_avl_node : public boost::intrusive::avl_set_base_hook<> {
public:
    using value_type = T;

    boost_avl_node(T x) : x_(x) {}

    friend auto operator<=>(boost_avl_node const& lhs, boost_avl_node const& rhs)
    {
        return lhs.x_ <=> rhs.x_;
    }

private:
    T x_;
};

constexpr auto a = sizeof(qct_node<int64_t>);
constexpr auto b = sizeof(boost_avl_node<int64_t>);
constexpr auto init_size = 100000;

static std::size_t seed()
{
    static auto seed = std::random_device{}();
    return seed;
}

template <template <typename...> typename TreeT, typename Node, typename Data>
static void BM_insert(benchmark::State& state)
{
    using Tree = TreeT<Node>;
    std::mt19937 gen(seed());
    std::uniform_int_distribution<Data> distrib(
        std::numeric_limits<Data>::min(), std::numeric_limits<Data>::max());

    std::list<Node> nodes;
    Tree tree;

    for (std::size_t i = 0; i < init_size; ++i) {
        auto x = distrib(gen);
        if constexpr (std::same_as<Data, Node>) {
            tree.insert(x);
        }
        else {
            nodes.push_back(Node{x});
            tree.insert(nodes.back());
        }
    }

    for (auto _ : state) {
        auto x = distrib(gen);
        if constexpr (std::same_as<Data, Node>) {
            tree.insert(x);
        }
        else {
            nodes.push_back(Node{x});
            tree.insert(nodes.back());
        }

        benchmark::DoNotOptimize(tree.begin());

        state.PauseTiming();
        tree.erase(*tree.find(x));
        if constexpr (!std::same_as<Data, Node>) {
            nodes.pop_back();
        }
        state.ResumeTiming();
    }
}

template <template <typename...> typename TreeT, typename Node, typename Data>
static void BM_erase(benchmark::State& state)
{
    using Tree = TreeT<Node>;
    std::mt19937 gen(seed());
    std::uniform_int_distribution<Data> distrib(
        std::numeric_limits<Data>::min(), std::numeric_limits<Data>::max());

    std::list<Node> nodes;
    Tree tree;

    for (std::size_t i = 0; i < init_size; ++i) {
        auto x = distrib(gen);
        if constexpr (std::same_as<Data, Node>) {
            tree.insert(x);
        }
        else {
            nodes.push_back(Node{x});
            tree.insert(nodes.back());
        }
    }

    for (auto _ : state) {
        auto x = distrib(gen);
        auto it = tree.lower_bound(x);
        if (it != tree.end()) {
            tree.erase(*it);
        }

        benchmark::DoNotOptimize(tree.begin());

        state.PauseTiming();
        if (it != tree.end()) {
            if constexpr (std::same_as<Data, Node>) {
                tree.insert(x);
            }
            else {
                tree.insert(*it);
            }
        }
        state.ResumeTiming();
    }
}

template <template <typename...> typename TreeT, typename Node, typename Data>
static void BM_find(benchmark::State& state)
{
    using Tree = TreeT<Node>;
    std::mt19937 gen(seed());
    std::uniform_int_distribution<Data> distrib(
        std::numeric_limits<Data>::min(), std::numeric_limits<Data>::max());

    std::list<Node> nodes;
    Tree tree;

    for (std::size_t i = 0; i < init_size; ++i) {
        auto x = distrib(gen);
        if constexpr (std::same_as<Data, Node>) {
            tree.insert(x);
        }
        else {
            nodes.push_back(Node{x});
            tree.insert(nodes.back());
        }
    }

    for (auto _ : state) {
        auto x = distrib(gen);
        benchmark::DoNotOptimize(tree.find(x));
    }
}

template <template <typename...> typename TreeT, typename Node, typename Data>
static void BM_lower_bound(benchmark::State& state)
{
    using Tree = TreeT<Node>;
    std::mt19937 gen(seed());
    std::uniform_int_distribution<Data> distrib(
        std::numeric_limits<Data>::min(), std::numeric_limits<Data>::max());

    std::list<Node> nodes;
    Tree tree;

    for (std::size_t i = 0; i < init_size; ++i) {
        auto x = distrib(gen);
        if constexpr (std::same_as<Data, Node>) {
            tree.insert(x);
        }
        else {
            nodes.push_back(Node{x});
            tree.insert(nodes.back());
        }
    }

    for (auto _ : state) {
        auto x = distrib(gen);
        benchmark::DoNotOptimize(tree.lower_bound(x));
    }
}

template <template <typename...> typename TreeT, typename Node, typename Data>
static void BM_lower_bound_distance(benchmark::State& state)
{
    using Tree = TreeT<Node>;
    std::mt19937 gen(seed());
    std::uniform_int_distribution<Data> distrib(
        std::numeric_limits<Data>::min(), std::numeric_limits<Data>::max());

    std::list<Node> nodes;
    Tree tree;

    for (std::size_t i = 0; i < init_size; ++i) {
        auto x = distrib(gen);
        if constexpr (std::same_as<Data, Node>) {
            tree.insert(x);
        }
        else {
            nodes.push_back(Node{x});
            tree.insert(nodes.back());
        }
    }

    for (auto _ : state) {
        auto x = distrib(gen);
        using std::distance;
        benchmark::DoNotOptimize(distance(tree.begin(), tree.lower_bound(x)));
    }
}

template <template <typename...> typename TreeT, typename Node, typename Data>
static void BM_iter(benchmark::State& state)
{
    using Tree = TreeT<Node>;
    std::mt19937 gen(seed());
    std::uniform_int_distribution<Data> distrib(
        std::numeric_limits<Data>::min(), std::numeric_limits<Data>::max());

    std::list<Node> nodes;
    Tree tree;

    for (std::size_t i = 0; i < init_size; ++i) {
        auto x = distrib(gen);
        if constexpr (std::same_as<Data, Node>) {
            tree.insert(x);
        }
        else {
            nodes.push_back(Node{x});
            tree.insert(nodes.back());
        }
    }

    for (auto _ : state) {
        int x{};
        for (auto const& _ : tree) {
            x++;
        }
        benchmark::DoNotOptimize(x);
    }
}

static void BM_set_insert(benchmark::State& state)
{
    BM_insert<std::multiset, int64_t, int64_t>(state);
}
BENCHMARK(BM_set_insert);

static void BM_avl_insert(benchmark::State& state)
{
    BM_insert<qct::avl_tree, qct_node<int64_t>, int64_t>(state);
}
BENCHMARK(BM_avl_insert);

static void BM_boost_avl_insert(benchmark::State& state)
{
    BM_insert<boost::intrusive::avl_multiset, boost_avl_node<int64_t>, int64_t>(
        state);
}
BENCHMARK(BM_boost_avl_insert);

static void BM_boost_rb_insert(benchmark::State& state)
{
    BM_insert<boost::intrusive::multiset, boost_rb_node<int64_t>, int64_t>(state);
}
BENCHMARK(BM_boost_rb_insert);

static void BM_set_erase(benchmark::State& state)
{
    BM_erase<std::multiset, int64_t, int64_t>(state);
}
BENCHMARK(BM_set_erase);

static void BM_avl_erase(benchmark::State& state)
{
    BM_erase<qct::avl_tree, qct_node<int64_t>, int64_t>(state);
}
BENCHMARK(BM_avl_erase);

static void BM_boost_avl_erase(benchmark::State& state)
{
    BM_erase<boost::intrusive::avl_multiset, boost_avl_node<int64_t>, int64_t>(
        state);
}
BENCHMARK(BM_boost_avl_erase);

static void BM_boost_rb_erase(benchmark::State& state)
{
    BM_erase<boost::intrusive::multiset, boost_rb_node<int64_t>, int64_t>(state);
}
BENCHMARK(BM_boost_rb_erase);

static void BM_set_find(benchmark::State& state)
{
    BM_find<std::multiset, int64_t, int64_t>(state);
}
BENCHMARK(BM_set_find);

static void BM_avl_find(benchmark::State& state)
{
    BM_find<qct::avl_tree, qct_node<int64_t>, int64_t>(state);
}
BENCHMARK(BM_avl_find);

static void BM_boost_avl_find(benchmark::State& state)
{
    BM_find<boost::intrusive::avl_multiset, boost_avl_node<int64_t>, int64_t>(
        state);
}
BENCHMARK(BM_boost_avl_find);

static void BM_boost_rb_find(benchmark::State& state)
{
    BM_find<boost::intrusive::multiset, boost_rb_node<int64_t>, int64_t>(state);
}
BENCHMARK(BM_boost_rb_find);

static void BM_set_lower_bound(benchmark::State& state)
{
    BM_lower_bound<std::multiset, int64_t, int64_t>(state);
}
BENCHMARK(BM_set_lower_bound);

static void BM_avl_lower_bound(benchmark::State& state)
{
    BM_lower_bound<qct::avl_tree, qct_node<int64_t>, int64_t>(state);
}
BENCHMARK(BM_avl_lower_bound);

static void BM_boost_avl_lower_bound(benchmark::State& state)
{
    BM_lower_bound<boost::intrusive::avl_multiset, boost_avl_node<int64_t>, int64_t>(
        state);
}
BENCHMARK(BM_boost_avl_lower_bound);

static void BM_boost_rb_lower_bound(benchmark::State& state)
{
    BM_lower_bound<boost::intrusive::multiset, boost_rb_node<int64_t>, int64_t>(
        state);
}
BENCHMARK(BM_boost_rb_lower_bound);

static void BM_set_lower_bound_distance(benchmark::State& state)
{
    BM_lower_bound_distance<std::multiset, int64_t, int64_t>(state);
}
BENCHMARK(BM_set_lower_bound_distance);

static void BM_avl_lower_bound_distance(benchmark::State& state)
{
    BM_lower_bound_distance<qct::avl_tree, qct_node<int64_t>, int64_t>(state);
}
BENCHMARK(BM_avl_lower_bound_distance);

static void BM_boost_avl_lower_bound_distance(benchmark::State& state)
{
    BM_lower_bound_distance<boost::intrusive::avl_multiset, boost_avl_node<int64_t>, int64_t>(
        state);
}
BENCHMARK(BM_boost_avl_lower_bound_distance);

static void BM_boost_rb_lower_bound_distance(benchmark::State& state)
{
    BM_lower_bound_distance<boost::intrusive::multiset, boost_rb_node<int64_t>, int64_t>(
        state);
}
BENCHMARK(BM_boost_rb_lower_bound_distance);

static void BM_set_iter(benchmark::State& state)
{
    BM_iter<std::multiset, int64_t, int64_t>(state);
}
BENCHMARK(BM_set_iter);

static void BM_avl_iter(benchmark::State& state)
{
    BM_iter<qct::avl_tree, qct_node<int64_t>, int64_t>(state);
}
BENCHMARK(BM_avl_iter);

static void BM_boost_avl_iter(benchmark::State& state)
{
    BM_iter<boost::intrusive::avl_multiset, boost_avl_node<int64_t>, int64_t>(
        state);
}
BENCHMARK(BM_boost_avl_iter);

static void BM_boost_rb_iter(benchmark::State& state)
{
    BM_iter<boost::intrusive::multiset, boost_rb_node<int64_t>, int64_t>(state);
}
BENCHMARK(BM_boost_rb_iter);

BENCHMARK_MAIN();
