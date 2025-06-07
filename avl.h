#pragma once

#include <cmath>
#include <cstdint>
#include <utility>

namespace qct {
namespace algorithms {

constexpr bool bst_is_header(auto const* x)
{
    if (!x->parent()) {
        // header of empty tree
        return true;
    }
    if (!x->right() || !x->left()) {
        // header always has both right and left
        return false;
    }
    if (x->right() == x->left()) {
        // header of single node tree
        return true;
    }
    if (x->right()->parent() != x || x->left()->parent() != x) {
        // size > 1, header can't be the parent of both rightmost and leftmost
        return true;
    }
    return false;
}

constexpr bool bst_is_root(auto const* x)
{
    return x->parent()->parent() == x;
}

}

template <int SizeBits = 32, int BalanceBits = std::min(8, 64 - SizeBits)>
class avl_node {
public:
    static constexpr auto size_bits = SizeBits;
    static constexpr auto balance_bits = BalanceBits;

    template <typename T>
    friend class avl_tree;

    constexpr std::size_t distance_from_begin() const
    {
        avl_node const* x = this;

        if (algorithms::bst_is_header(x)) {
            if (!x->parent_) {
                return 0;
            }
            return x->right_->distance_from_begin() + 1;
        }

        std::size_t distance = 0;

        if (x->left_) {
            distance += x->left_->subtree_size_;
        }

        for (; !algorithms::bst_is_root(x); x = x->parent_) {
            if (x == x->parent_->right_) {
                distance += x->parent_->subtree_size_ - x->subtree_size_;
            }
        }
        return distance;
    }

    constexpr auto balance() const { return balance_; }
    constexpr auto subtree_size() const { return subtree_size_; }
    constexpr avl_node const* left() const { return left_; }
    constexpr avl_node const* right() const { return right_; }
    constexpr avl_node const* parent() const { return parent_; }

private:
    avl_node* parent_{nullptr};
    avl_node* left_{nullptr};
    avl_node* right_{nullptr};
    std::size_t subtree_size_ : SizeBits{0};
    int8_t balance_ : BalanceBits{0};
};

template <typename Node>
class avl_tree {
    using avl_node = std::remove_pointer_t<decltype(Node::parent_)>;

    template <bool Const>
    class iterator_impl {
    public:
        using difference_type = std::make_signed_t<std::size_t>;
        using value_type = std::conditional_t<Const, Node const, Node>;

        constexpr iterator_impl() = default;
        constexpr iterator_impl(avl_node* node) : node_{node} {}

        constexpr operator iterator_impl<true>() const
            requires(!Const)
        {
            return iterator_impl<true>{node_};
        }

        constexpr iterator_impl& operator++()
        {
            node_ = bst_successor(node_);
            return *this;
        }
        constexpr iterator_impl operator++(int)
        {
            iterator_impl retval = *this;
            ++(*this);
            return retval;
        }
        constexpr iterator_impl& operator--()
        {
            node_ = bst_predecessor(node_);
            return *this;
        }
        constexpr iterator_impl operator--(int)
        {
            iterator_impl retval = *this;
            ++(*this);
            return retval;
        }
        constexpr bool operator==(iterator_impl other) const
        {
            return node_ == other.node_;
        }
        constexpr bool operator!=(iterator_impl other) const
        {
            return !(*this == other);
        }
        constexpr value_type& operator*() const { return *upcast(node_); }
        constexpr value_type* operator->() const { return upcast(node_); }

        constexpr friend difference_type distance(iterator_impl lhs, iterator_impl rhs)
        {
            return (rhs.node_ ? rhs.node_->distance_from_begin() : 0)
                   - (lhs.node_ ? lhs.node_->distance_from_begin() : 0);
        }

    private:
        friend class avl_tree;

        iterator_impl<false> as_mutable() const
            requires(Const)
        {
            return iterator_impl<false>{const_cast<Node*>(node_)};
        }

        avl_node* node_{nullptr};
    };

public:
    using value_type = Node;
    using iterator = iterator_impl<false>;
    using const_iterator = iterator_impl<true>;

    static_assert(std::bidirectional_iterator<iterator>);
    static_assert(std::bidirectional_iterator<const_iterator>);

    constexpr iterator begin() { return iterator{leftmost()}; }
    constexpr iterator end() { return iterator{&header_}; }
    constexpr const_iterator begin() const { return as_mutable().begin(); }
    constexpr const_iterator end() const { return as_mutable().end(); }
    constexpr std::size_t size() const
    {
        return root() ? root()->subtree_size_ : 0;
    }

    constexpr void erase(Node const& node)
    {
        auto info = bst_erase(&node);
        if (info.y) {
            info.y->balance_ = node.balance_;
            info.y->subtree_size_ = node.subtree_size_;
        }
        avl_erase_rebalance(info);
    }

    constexpr void insert(Node& node)
    {
        bst_insert(node);
        avl_insert_rebalance(&node);
    }

    template <typename T>
    constexpr iterator lower_bound(T const& val)
    {
        iterator res = end();
        Node* current = upcast(root());
        while (current) {
            if (*current < val) {
                current = upcast(current->right_);
            }
            else {
                res = iterator{current};
                current = upcast(current->left_);
            }
        }
        return res;
    }

    template <typename T>
    constexpr const_iterator lower_bound(T const& val) const
    {
        return as_mutable().lower_bound(val);
    }

    template <typename T>
    constexpr iterator find(T const& val)
    {
        auto lb = lower_bound(val);
        return lb == end() || *lb < val ? end() : lb;
    }

    template <typename T>
    constexpr const_iterator find(T const& val) const
    {
        return as_mutable().find(val);
    }

private:
    struct erase_rebalance_info {
        avl_node* x{};
        avl_node* y{};
        bool n_is_left{};
    };

    static constexpr Node* upcast(avl_node* p) { return static_cast<Node*>(p); }

    static constexpr avl_node* bst_minimum(avl_node* x)
    {
        while (x->left_) {
            x = x->left_;
        }
        return x;
    }

    static constexpr avl_node* bst_maximum(avl_node* x)
    {
        while (x->right_) {
            x = x->right_;
        }
        return x;
    }

    static constexpr avl_node* bst_successor(avl_node* x)
    {
        if (x->right_) {
            return bst_minimum(x->right_);
        }
        avl_node* y = x->parent_;
        while (x == y->right_) {
            x = y;
            y = y->parent_;
        }
        if (x->right_ == y) {
            // x is the header
            return x;
        }
        return y;
    }

    static constexpr avl_node* bst_predecessor(avl_node* x)
    {
        if (algorithms::bst_is_header(x)) {
            return bst_maximum(x->parent_);
        }
        if (x->left_) {
            return bst_maximum(x->left_);
        }
        avl_node* y = x->parent_;
        while (x == y->left_) {
            x = y;
            y = y->parent_;
        }
        return y;
    }

    constexpr void bst_insert(Node& node)
    {
        bool is_first = true;
        bool is_last = true;
        avl_node* parent = &header_;
        avl_node** current = &root();
        while (*current) {
            parent = *current;
            parent->subtree_size_++;
            if (*upcast(*current) < node) {
                current = &(*current)->right_;
                is_first = false;
            }
            else {
                current = &(*current)->left_;
                is_last = false;
            }
        }
        *current = &node;

        node.parent_ = parent;
        node.left_ = nullptr;
        node.right_ = nullptr;
        node.balance_ = 0;
        node.subtree_size_ = 1;

        if (is_first) {
            leftmost() = &node;
        }
        if (is_last) {
            rightmost() = &node;
        }
    }

    constexpr void bst_shift_nodes(avl_node const* u, avl_node* v)
    {
        if (u == root()) {
            root() = v;
        }
        else if (u == u->parent_->left_) {
            u->parent_->left_ = v;
        }
        else {
            u->parent_->right_ = v;
        }
        if (v) {
            v->parent_ = u->parent_;
        }
    }

    constexpr erase_rebalance_info bst_erase(avl_node const* z)
    {
        erase_rebalance_info info{};
        info.x = z->parent_;
        info.n_is_left = z->parent_ && z == z->parent_->left_;
        if (!z->left_) {
            bst_shift_nodes(z, z->right_);
            if (z == leftmost()) {
                leftmost() = z->right_;
            }
        }
        else if (!z->right_) {
            bst_shift_nodes(z, z->left_);
            if (z == rightmost()) {
                rightmost() = z->left_;
            }
        }
        else {
            avl_node* y = bst_minimum(z->right_);
            if (y->parent_ != z) {
                info.x = y->parent_;
                info.n_is_left = true;
                bst_shift_nodes(y, y->right_);

                y->right_ = z->right_;
                y->right_->parent_ = y;
            }
            else {
                info.x = y;
                info.n_is_left = false;
            }
            bst_shift_nodes(z, y);
            y->left_ = z->left_;
            y->left_->parent_ = y;
            info.y = y;
        }
        return info;
    }

    static constexpr avl_node* rotate_left(avl_node* x, avl_node* z)
    {
        avl_node* tmp = z->left_;
        x->right_ = tmp;
        if (tmp) {
            tmp->parent_ = x;
        }
        z->left_ = x;
        x->parent_ = z;

        auto x_subtree_size = x->subtree_size_;
        x->subtree_size_ -= z->subtree_size_ - (tmp ? tmp->subtree_size_ : 0);
        z->subtree_size_ = x_subtree_size;

        if (z->balance_ == 0) {
            x->balance_ = 1;
            z->balance_ = -1;
        }
        else {
            x->balance_ = 0;
            z->balance_ = 0;
        }
        return z;
    }

    static constexpr avl_node* rotate_right(avl_node* x, avl_node* z)
    {
        avl_node* tmp = z->right_;
        x->left_ = tmp;
        if (tmp) {
            tmp->parent_ = x;
        }
        z->right_ = x;
        x->parent_ = z;

        auto x_subtree_size = x->subtree_size_;
        x->subtree_size_ -= z->subtree_size_ - (tmp ? tmp->subtree_size_ : 0);
        z->subtree_size_ = x_subtree_size;

        if (z->balance_ == 0) {
            x->balance_ = -1;
            z->balance_ = 1;
        }
        else {
            x->balance_ = 0;
            z->balance_ = 0;
        }
        return z;
    }

    static constexpr avl_node* rotate_right_left(avl_node* x, avl_node* z)
    {
        avl_node* y = z->left_;
        avl_node* tmp1 = y->right_;
        z->left_ = tmp1;
        if (tmp1) {
            tmp1->parent_ = z;
        }
        y->right_ = z;
        z->parent_ = y;

        avl_node* tmp2 = y->left_;
        x->right_ = tmp2;
        if (tmp2) {
            tmp2->parent_ = x;
        }
        y->left_ = x;
        x->parent_ = y;

        auto x_subtree_size = x->subtree_size_;
        x->subtree_size_ -= z->subtree_size_ - (tmp2 ? tmp2->subtree_size_ : 0);
        z->subtree_size_ -= y->subtree_size_ - (tmp1 ? tmp1->subtree_size_ : 0);
        y->subtree_size_ = x_subtree_size;

        if (y->balance_ == 0) {
            x->balance_ = 0;
            z->balance_ = 0;
        }
        else if (y->balance_ > 0) {
            x->balance_ = -1;
            z->balance_ = 0;
        }
        else {
            x->balance_ = 0;
            z->balance_ = 1;
        }
        y->balance_ = 0;
        return y;
    }

    static constexpr avl_node* rotate_left_right(avl_node* x, avl_node* z)
    {
        avl_node* y = z->right_;
        avl_node* tmp1 = y->left_;
        z->right_ = tmp1;
        if (tmp1) {
            tmp1->parent_ = z;
        }
        y->left_ = z;
        z->parent_ = y;

        avl_node* tmp2 = y->right_;
        x->left_ = tmp2;
        if (tmp2) {
            tmp2->parent_ = x;
        }
        y->right_ = x;
        x->parent_ = y;

        auto x_subtree_size = x->subtree_size_;
        x->subtree_size_ -= z->subtree_size_ - (tmp2 ? tmp2->subtree_size_ : 0);
        z->subtree_size_ -= y->subtree_size_ - (tmp1 ? tmp1->subtree_size_ : 0);
        y->subtree_size_ = x_subtree_size;

        if (y->balance_ == 0) {
            x->balance_ = 0;
            z->balance_ = 0;
        }
        else if (y->balance_ < 0) {
            x->balance_ = 1;
            z->balance_ = 0;
        }
        else {
            x->balance_ = 0;
            z->balance_ = -1;
        }
        y->balance_ = 0;
        return y;
    }

    constexpr void avl_erase_rebalance(erase_rebalance_info info)
    {
        avl_node* g;
        avl_node* n;
        bool n_is_left = info.n_is_left;
        bool height_changed = false;

        for (avl_node* x = info.x; x != &header_;
             x = g, n_is_left = x && n == x->left_) {
            g = x->parent_;
            x->subtree_size_--;

            if (n_is_left) {
                if (x->balance_ > 0) {
                    avl_node* z = x->right_;
                    height_changed = z->balance_ != 0;
                    if (z->balance_ < 0) {
                        n = rotate_right_left(x, z);
                    }
                    else {
                        n = rotate_left(x, z);
                    }
                }
                else if (x->balance_ < 0) {
                    x->balance_ = 0;
                    n = x;
                    continue;
                }
                else {
                    x->balance_ = 1;
                    break;
                }
            }
            else {
                if (x->balance_ < 0) {
                    avl_node* z = x->left_;
                    height_changed = z->balance_ != 0;
                    if (z->balance_ > 0) {
                        n = rotate_left_right(x, z);
                    }
                    else {
                        n = rotate_right(x, z);
                    }
                }
                else if (x->balance_ > 0) {
                    x->balance_ = 0;
                    n = x;
                    continue;
                }
                else {
                    x->balance_ = -1;
                    break;
                }
            }

            n->parent_ = g;
            if (g != &header_) {
                if (x == g->left_) {
                    g->left_ = n;
                }
                else {
                    g->right_ = n;
                }
            }
            else {
                root() = n;
            }
            if (!height_changed) {
                break;
            }
        }

        for (avl_node* x = g; x != &header_; x = x->parent_) {
            x->subtree_size_--;
        }
    }

    constexpr void avl_insert_rebalance(avl_node* z)
    {
        for (avl_node* x = z->parent_; x != &header_; z = x, x = z->parent_) {
            avl_node* n;
            avl_node* g = x->parent_;
            if (z == x->left_) {
                if (x->balance_ < 0) {
                    if (z->balance_ > 0) {
                        n = rotate_left_right(x, z);
                    }
                    else {
                        n = rotate_right(x, z);
                    }
                }
                else if (x->balance_ > 0) {
                    x->balance_ = 0;
                    break;
                }
                else {
                    x->balance_ = -1;
                    continue;
                }
            }
            else {
                if (x->balance_ > 0) {
                    if (z->balance_ < 0) {
                        n = rotate_right_left(x, z);
                    }
                    else {
                        n = rotate_left(x, z);
                    }
                }
                else if (x->balance_ < 0) {
                    x->balance_ = 0;
                    break;
                }
                else {
                    x->balance_ = 1;
                    continue;
                }
            }

            n->parent_ = g;
            if (g != &header_) {
                if (x == g->left_) {
                    g->left_ = n;
                }
                else {
                    g->right_ = n;
                }
            }
            else {
                root() = n;
            }
            break;
        }
    }

    constexpr avl_node*& root() { return header_.parent_; }
    constexpr avl_node* root() const { return header_.parent_; }
    constexpr avl_node*& leftmost() { return header_.left_; }
    constexpr avl_node*& rightmost() { return header_.right_; }

    constexpr avl_tree& as_mutable() const
    {
        return *const_cast<avl_tree*>(this);
    }

    avl_node header_;
};

}
