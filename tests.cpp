#include <algorithm>
#include <forward_list>
#include <random>
#include <ranges>

#include <catch2/catch_template_test_macros.hpp>

#include "qct.h"

struct node : public qct::node<> {
public:
    constexpr explicit node(int x) : x_(x) {}

    constexpr int const& data() const { return x_; }

private:
    int x_;
};

struct node_comparator {
    constexpr bool operator()(node const& lhs, node const& rhs) const
    {
        return lhs.data() < rhs.data();
    }
    constexpr bool operator()(int lhs, node const& rhs) const
    {
        return lhs < rhs.data();
    }
    constexpr bool operator()(node const& lhs, int rhs) const
    {
        return lhs.data() < rhs;
    }
};

class comparable_node : public qct::node<> {
public:
    explicit comparable_node(int x) : x_(x) {}

    constexpr auto operator<=>(comparable_node const&) const = default;
    friend auto operator<=>(comparable_node const& lhs, int rhs)
    {
        return lhs.x_ <=> rhs;
    }

    int const& data() const { return x_; }

private:
    int x_;
};

constexpr auto seed = 43;

template <typename Tree>
void check_invariants(Tree const& tree)
{
    using Node = typename Tree::value_type;
    using Comparator = typename Tree::value_compare;

    CHECK(tree.begin()->distance_from_begin() == 0);
    CHECK(tree.end()->distance_from_begin() == tree.size());

    CHECK(tree.size() == std::ranges::distance(tree));
    CHECK(distance(tree.begin(), tree.end()) == std::ranges::distance(tree));
    CHECK(std::ranges::is_sorted(tree, Comparator{}));

    CHECK(
        tree.size()
        == std::distance(
            std::make_reverse_iterator(tree.end()),
            std::make_reverse_iterator(tree.begin())));

    CHECK(std::ranges::is_sorted(
        std::make_reverse_iterator(tree.end()),
        std::make_reverse_iterator(tree.begin()),
        [](auto const& lhs, auto const& rhs) { return Comparator{}(rhs, lhs); }));

    for (auto const& n : tree) {
        if (n.right()) {
            CHECK(!Comparator{}(*static_cast<Node const*>(n.right()), n));
        }
        if (n.left()) {
            CHECK(!Comparator{}(n, *static_cast<Node const*>(n.left())));
        }

        if (n.balance() == 1) {
            CHECK(n.right());
        }
        else if (n.balance() == -1) {
            CHECK(n.left());
        }
        else {
            CHECK(n.balance() == 0);
            CHECK((!!n.left()) == (!!n.right()));
        }

        CHECK(
            n.subtree_size()
            == 1 + (n.left() ? n.left()->subtree_size() : 0)
                   + (n.right() ? n.right()->subtree_size() : 0));
    }
}

TEMPLATE_TEST_CASE(
    "Insert/Erase",
    "[insert][erase]",
    (std::pair<comparable_node, std::less<>>),
    (std::pair<comparable_node, std::greater<>>),
    (std::pair<node, node_comparator>))
{
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distrib(
        std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

    using Node = typename TestType::first_type;
    using Comparator = typename TestType::second_type;
    using Tree = qct::tree<Node, Comparator>;

    auto const n = 1000000;
    std::vector<Node> nodes;
    nodes.reserve(n);
    Tree tree;

    for (int i = 0; i < n; ++i) {
        auto x = distrib(gen);
        nodes.push_back(Node{x});
        tree.insert(nodes.back());
        CHECK(tree.size() == i + 1);
    }
    check_invariants(tree);

    int erased = 0;
    for (int i = 0; i < n / 2; ++i) {
        auto x = distrib(gen);
        auto it = tree.lower_bound(x);
        if (it != tree.end()) {
            tree.erase(*it);
            erased++;
            CHECK(tree.size() == n - erased);
        }
    }

    check_invariants(tree);
}

TEMPLATE_TEST_CASE(
    "Find",
    "[find]",
    (std::pair<comparable_node, std::less<>>),
    (std::pair<comparable_node, std::greater<>>),
    (std::pair<node, node_comparator>))
{
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distrib(
        std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

    using Node = typename TestType::first_type;
    using Comparator = typename TestType::second_type;
    using Tree = qct::tree<Node, Comparator>;

    auto const n = 1000000;
    std::vector<Node> nodes;
    nodes.reserve(n);
    Tree tree;

    for (int i = 0; i < n; ++i) {
        auto x = distrib(gen);
        nodes.push_back(Node{x});
        tree.insert(nodes.back());
    }
    auto const min = *tree.begin();
    auto const max = *std::prev(tree.end());

    CHECK(tree.find(min) == tree.begin());
    CHECK(tree.find(max) == std::prev(tree.end()));

    int x = 0;
    for (int i = 0; i < n; ++i) {
        if (x % 2 == 0) {
            // for half of the iteration, make sure we pick a value that exists
            x = nodes[distrib(gen) % nodes.size()].data();
        }
        else {
            x = distrib(gen);
        }
        auto lb = tree.lower_bound(x);
        auto ub = tree.upper_bound(x);
        auto f = tree.find(x);

        if (lb == tree.end()) {
            CHECK(f == tree.end());
            CHECK(lb == ub);
            CHECK(Comparator{}(max, x));
        }
        else if (ub == tree.begin()) {
            CHECK(f == tree.end());
            CHECK(lb == ub);
            CHECK(Comparator{}(x, min));
        }
        else if (lb == ub) {
            CHECK(f == tree.end());
            CHECK(!Comparator{}(*lb, x));
            CHECK(Comparator{}(x, *ub));
        }
        else {
            CHECK(f != tree.end());
            CHECK((!Comparator{}(*f, x) && !Comparator{}(x, *f))); // ==
            CHECK(!Comparator{}(*lb, x));
            if (ub != tree.end()) {
                CHECK(Comparator{}(x, *ub));
            }
            CHECK(f == lb);
            for (auto it = lb; it != ub; ++it) {
                CHECK((!Comparator{}(x, *it) && !Comparator{}(x, *it))); // ==
            }
        }
    }
}
