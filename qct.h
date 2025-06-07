#pragma once

#include <functional>
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
class node {
public:
    static constexpr auto size_bits = SizeBits;
    static constexpr auto balance_bits = BalanceBits;

    template <typename T, typename Comp>
    friend class tree;

    constexpr auto operator<=>(node const&) const { return true <=> true; }

    constexpr std::size_t distance_from_begin() const
    {
        node const* x = this;

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
    constexpr node const* left() const { return left_; }
    constexpr node const* right() const { return right_; }
    constexpr node const* parent() const { return parent_; }

private:
    node* parent_{nullptr};
    node* left_{nullptr};
    node* right_{nullptr};
    std::size_t subtree_size_ : SizeBits{0};
    int8_t balance_ : BalanceBits{0};
};

template <typename Node, typename Comp = std::less<>>
class tree {
    using node = std::remove_pointer_t<decltype(Node::parent_)>;

    template <bool Const>
    class iterator_impl {
    public:
        using difference_type = std::make_signed_t<std::size_t>;
        using value_type = std::conditional_t<Const, Node const, Node>;

        constexpr iterator_impl() = default;
        constexpr iterator_impl(node* node) : node_{node} {}

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
        friend class tree;

        iterator_impl<false> as_mutable() const
            requires(Const)
        {
            return iterator_impl<false>{const_cast<Node*>(node_)};
        }

        node* node_{nullptr};
    };

public:
    using value_type = Node;
    using value_compare = Comp;
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
        qct_erase_rebalance(info);
    }

    constexpr void insert(Node& node)
    {
        qct_insert(node);
        qct_insert_rebalance(&node);
    }

    template <typename T>
    constexpr iterator lower_bound(T const& val)
    {
        iterator res = end();
        Node* current = upcast(root());
        while (current) {
            if (Comp{}(*current, val)) {
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
    constexpr iterator upper_bound(T const& val)
    {
        iterator res = end();
        Node* current = upcast(root());
        while (current) {
            if (Comp{}(val, *current)) {
                res = iterator{current};
                current = upcast(current->left_);
            }
            else {
                current = upcast(current->right_);
            }
        }
        return res;
    }

    template <typename T>
    constexpr const_iterator upper_bound(T const& val) const
    {
        return as_mutable().upper_bound(val);
    }

    template <typename T>
    constexpr iterator find(T const& val)
    {
        auto lb = lower_bound(val);
        return lb == end() || Comp{}(val, *lb) ? end() : lb;
    }

    template <typename T>
    constexpr const_iterator find(T const& val) const
    {
        return as_mutable().find(val);
    }

private:
    struct erase_rebalance_info {
        node* x{};
        node* y{};
        bool n_is_left{};
    };

    static constexpr Node* upcast(node* p) { return static_cast<Node*>(p); }

    static constexpr node* bst_minimum(node* x)
    {
        while (x->left_) {
            x = x->left_;
        }
        return x;
    }

    static constexpr node* bst_maximum(node* x)
    {
        while (x->right_) {
            x = x->right_;
        }
        return x;
    }

    static constexpr node* bst_successor(node* x)
    {
        if (x->right_) {
            return bst_minimum(x->right_);
        }
        node* y = x->parent_;
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

    static constexpr node* bst_predecessor(node* x)
    {
        if (algorithms::bst_is_header(x)) {
            return bst_maximum(x->parent_);
        }
        if (x->left_) {
            return bst_maximum(x->left_);
        }
        node* y = x->parent_;
        while (x == y->left_) {
            x = y;
            y = y->parent_;
        }
        return y;
    }

    constexpr void bst_shift_nodes(node const* u, node* v)
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

    constexpr void qct_insert(Node& x)
    {
        bool is_leftmost = true;
        bool is_rightmost = true;
        node* parent = &header_;
        node** current = &root();
        while (*current) {
            parent = *current;
            parent->subtree_size_++;
            if (Comp{}(*upcast(*current), x)) {
                current = &(*current)->right_;
                is_leftmost = false;
            }
            else {
                current = &(*current)->left_;
                is_rightmost = false;
            }
        }
        *current = &x;

        x.parent_ = parent;
        x.left_ = nullptr;
        x.right_ = nullptr;
        x.balance_ = 0;
        x.subtree_size_ = 1;

        if (is_leftmost) {
            leftmost() = &x;
        }
        if (is_rightmost) {
            rightmost() = &x;
        }
    }

    constexpr erase_rebalance_info bst_erase(node const* z)
    {
        erase_rebalance_info info{};
        info.x = z->parent_;
        info.n_is_left = z->parent_ && z == z->parent_->left_;
        if (!z->left_) {
            bst_shift_nodes(z, z->right_);
            if (z == leftmost()) {
                // z->right could be null
                leftmost() = bst_minimum(z->parent_);
            }
        }
        else if (!z->right_) {
            bst_shift_nodes(z, z->left_);
            if (z == rightmost()) {
                // z->left can't be null
                rightmost() = bst_maximum(z->left_);
            }
        }
        else {
            node* y = bst_minimum(z->right_);
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

    static constexpr node* qct_rotate_left(node* x, node* z)
    {
        node* tmp = z->left_;
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

    static constexpr node* qct_rotate_right(node* x, node* z)
    {
        node* tmp = z->right_;
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

    static constexpr node* qct_rotate_right_left(node* x, node* z)
    {
        node* y = z->left_;
        node* tmp1 = y->right_;
        z->left_ = tmp1;
        if (tmp1) {
            tmp1->parent_ = z;
        }
        y->right_ = z;
        z->parent_ = y;

        node* tmp2 = y->left_;
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

    static constexpr node* qct_rotate_left_right(node* x, node* z)
    {
        node* y = z->right_;
        node* tmp1 = y->left_;
        z->right_ = tmp1;
        if (tmp1) {
            tmp1->parent_ = z;
        }
        y->left_ = z;
        z->parent_ = y;

        node* tmp2 = y->right_;
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

    constexpr void qct_insert_rebalance(node* z)
    {
        for (node* x = z->parent_; x != &header_; z = x, x = z->parent_) {
            node* n;
            node* g = x->parent_;
            if (z == x->left_) {
                if (x->balance_ < 0) {
                    if (z->balance_ > 0) {
                        n = qct_rotate_left_right(x, z);
                    }
                    else {
                        n = qct_rotate_right(x, z);
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
                        n = qct_rotate_right_left(x, z);
                    }
                    else {
                        n = qct_rotate_left(x, z);
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

    constexpr void qct_erase_rebalance(erase_rebalance_info info)
    {
        node* g;
        node* n;
        bool n_is_left = info.n_is_left;
        bool height_changed = false;

        for (node* x = info.x; x != &header_;
             x = g, n_is_left = x && n == x->left_) {
            g = x->parent_;
            x->subtree_size_--;

            if (n_is_left) {
                if (x->balance_ > 0) {
                    node* z = x->right_;
                    height_changed = z->balance_ != 0;
                    if (z->balance_ < 0) {
                        n = qct_rotate_right_left(x, z);
                    }
                    else {
                        n = qct_rotate_left(x, z);
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
                    node* z = x->left_;
                    height_changed = z->balance_ != 0;
                    if (z->balance_ > 0) {
                        n = qct_rotate_left_right(x, z);
                    }
                    else {
                        n = qct_rotate_right(x, z);
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

        for (node* x = g; x != &header_; x = x->parent_) {
            x->subtree_size_--;
        }
    }

    constexpr node*& root() { return header_.parent_; }
    constexpr node* root() const { return header_.parent_; }
    constexpr node*& leftmost() { return header_.left_; }
    constexpr node*& rightmost() { return header_.right_; }

    constexpr tree& as_mutable() const { return *const_cast<tree*>(this); }

    node header_;
};

}
