#include <algorithm>
#include <forward_list>
#include <random>

#include <catch2/catch_test_macros.hpp>

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

    T const& data() const { return x_; }

private:
    T x_;
};

constexpr auto seed = 43;

template <typename Tree>
void check_invariants(Tree const& tree)
{
    using Node = typename Tree::value_type;

    CHECK(tree.begin()->distance_from_begin() == 0);
    CHECK(tree.end()->distance_from_begin() == tree.size());

    CHECK(tree.size() == std::distance(tree.begin(), tree.end()));
    CHECK(
        distance(tree.begin(), tree.end())
        == std::distance(tree.begin(), tree.end()));

    CHECK(std::is_sorted(tree.begin(), tree.end()));

    for (auto const& n : tree) {
        if (n.right()) {
            REQUIRE(n.data() <= static_cast<Node const*>(n.right())->data());
        }
        if (n.left()) {
            REQUIRE(static_cast<Node const*>(n.left())->data() <= n.data());
        }

        if (n.balance() == 1) {
            REQUIRE(n.right());
        }
        else if (n.balance() == -1) {
            REQUIRE(n.left());
        }
        else {
            REQUIRE(n.balance() == 0);
            REQUIRE((!!n.left()) == (!!n.right()));
        }

        REQUIRE(
            n.subtree_size()
            == 1 + (n.left() ? n.left()->subtree_size() : 0)
                   + (n.right() ? n.right()->subtree_size() : 0));
    }
}

TEST_CASE("Insert", "[insert]")
{
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distrib(
        std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

    using Node = qct_node<int>;
    using Tree = qct::avl_tree<Node>;

    std::forward_list<Node> nodes;
    Tree tree;

    auto const n = 1000000;
    for (int i = 0; i < n; ++i) {
        auto x = distrib(gen);
        nodes.push_front(Node{x});
        tree.insert(nodes.front());
        CHECK(tree.size() == i + 1);
    }
    check_invariants(tree);
}

TEST_CASE("Erase", "[erase]")
{
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distrib(
        std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

    using Node = qct_node<int>;
    using Tree = qct::avl_tree<Node>;

    std::forward_list<Node> nodes;
    Tree tree;

    auto const n = 1000000;
    for (int i = 0; i < n; ++i) {
        auto x = distrib(gen);
        nodes.push_front(Node{x});
        tree.insert(nodes.front());
        CHECK(tree.size() == i + 1);
    }

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
