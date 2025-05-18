#pragma once

#include <cstdint>
#include <iostream>
#include <list>
#include <queue>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

template <typename Derived>
class AVLTree;

template <typename Derived>
class Node {
public:
    friend class AVLTree<Derived>;

    friend std::ostream& operator<<(std::ostream& os, Node<Derived> const& node)
    {
        return os << "Node<" << node.data() << ", "
                  << static_cast<int>(node.balance_) << ", "
                  << static_cast<int>(node.subtree_size) << ">";
    }

private:
    Derived const& data() const { return *static_cast<Derived const*>(this); }

    Node* parent_{nullptr};
    Node* left_{nullptr};
    Node* right_{nullptr};
    std::size_t subtree_size{0};
    int8_t balance_{0};
};

template <typename Derived>
class AVLTree {
public:
    using iterator = Node<Derived>*;

    iterator begin() const { return begin_; }
    iterator end() const { return nullptr; }

    iterator lower_bound(Derived const& val) const
    {
        Node<Derived>* current = root_;
        while (true) {
            if (val < current->data()) {
                if (!current->left_) {
                    return current;
                }
                current = current->left_;
            }
            else if (current->data() < val) {
                if (!current->right_) {
                    return current;
                }
                current = current->right_;
            }
            else {
                return current;
            }
        }
    }

    iterator insert(Node<Derived>& node)
    {
        bool is_begin = true;
        Node<Derived>* parent = nullptr;
        Node<Derived>** current = &root_;
        while (*current) {
            parent = *current;
            parent->subtree_size++;
            if ((*current)->data() < node.data()) {
                current = &(*current)->right_;
                is_begin = false;
            }
            else {
                current = &(*current)->left_;
            }
        }
        *current = &node;

        node.parent_ = parent;
        node.left_ = nullptr;
        node.right_ = nullptr;
        node.balance_ = 0;
        node.subtree_size = 1;
        if (is_begin) {
            begin_ = &node;
        }

        insert_rebalance_from(node);
        return &node;
    }

    static std::size_t distance_from_begin(iterator it)
    {
        std::size_t distance = 0;
        auto z = it;
        for (auto x = it->parent_; x; z = x, x = x->parent_) {
            if (z == x->right_) {
                distance += (x->left_ ? x->left_->subtree_size : 0) + 1;
            }
        }
        return distance;
    }

    static std::make_signed_t<std::size_t> distance(iterator lhs, iterator rhs)
    {
        return distance_from_begin(rhs) - distance_from_begin(lhs);
    }

    template <typename Visitor>
    void bf_visit(Visitor const& visitor, bool visit_empty = false) const
    {
        std::queue<std::pair<std::size_t, iterator>> fifo;
        if (root_) {
            fifo.push({0, root_});
        }
        bool level_empty = true;
        std::size_t current_level = 0;
        while (!fifo.empty()) {
            auto const [level, node] = std::move(fifo.front());
            fifo.pop();
            if (current_level != level) {
                if (level_empty) {
                    break;
                }
                current_level = level;
                level_empty = true;
            }
            visitor(level, node);

            if (node) {
                if (node->left_ || visit_empty) {
                    fifo.push({level + 1, node->left_});
                    if (node->left_)
                        level_empty = false;
                }
                if (node->right_ || visit_empty) {
                    fifo.push({level + 1, node->right_});
                    if (node->right_)
                        level_empty = false;
                }
            }
            else if (visit_empty) {
                fifo.push({level + 1, nullptr});
                fifo.push({level + 1, nullptr});
            }
        }
    }

    friend std::ostream& operator<<(std::ostream& os, AVLTree<Derived> const& tree)
    {
        std::size_t current_level = 0;
        std::string sep = "";
        std::size_t size = 0;
        tree.bf_visit(
            [&](std::size_t level, iterator node) {
                if (level != current_level) {
                    os << '\n';
                    current_level = level;
                    sep = "";
                }

                os << std::exchange(sep, " | ");
                if (node) {
                    if (!size) {
                        std::stringstream ss;
                        ss << *node;
                        size = ss.str().size();
                    }
                    os << *node;
                }
                else {
                    os << "<Empty>" << std::string(size > 7 ? size - 7 : 0, ' ');
                }
            },
            true);
        return os;
    }

private:
    void insert_rebalance_from(Node<Derived>& node)
    {
        Node<Derived>* z = &node;
        for (Node<Derived>* x = node.parent_; x; z = x, x = x->parent_) {
            Node<Derived>* n;
            Node<Derived>* g = x->parent_;
            if (z == x->left_) {
                if (x->balance_ >= 0) {
                    x->balance_--;
                    if (x->balance_ == 0) {
                        break;
                    }
                    continue;
                }

                if (z->balance_ <= 0) {
                    n = rotate_right(x, z);
                }
                else {
                    n = rotate_left_right(x, z);
                }
            }
            else {
                if (x->balance_ <= 0) {
                    x->balance_++;
                    if (x->balance_ == 0) {
                        break;
                    }
                    continue;
                }

                if (z->balance_ >= 0) {
                    n = rotate_left(x, z);
                }
                else {
                    n = rotate_right_left(x, z);
                }
            }

            n->parent_ = g;
            if (g) {
                if (x == g->left_) {
                    g->left_ = n;
                }
                else {
                    g->right_ = n;
                }
            }
            else {
                root_ = n;
            }
            break;
        }
    }

    static Node<Derived>* rotate_left(Node<Derived>* x, Node<Derived>* z)
    {
        Node<Derived>* tmp = z->left_;

        x->right_ = tmp;
        if (tmp) {
            tmp->parent_ = x;
        }
        z->left_ = x;
        x->parent_ = z;

        auto x_subtree_size = x->subtree_size;
        x->subtree_size -= z->subtree_size - (tmp ? tmp->subtree_size : 0);
        z->subtree_size = x_subtree_size;

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
    static Node<Derived>* rotate_right(Node<Derived>* x, Node<Derived>* z)
    {
        Node<Derived>* tmp = z->right_;
        x->left_ = tmp;
        if (tmp) {
            tmp->parent_ = x;
        }
        z->right_ = x;
        x->parent_ = z;

        auto x_subtree_size = x->subtree_size;
        x->subtree_size -= z->subtree_size - (tmp ? tmp->subtree_size : 0);
        z->subtree_size = x_subtree_size;

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

    static Node<Derived>* rotate_right_left(Node<Derived>* x, Node<Derived>* z)
    {
        Node<Derived>* y = z->left_;
        Node<Derived>* tmp1 = y->right_;
        z->left_ = tmp1;
        if (tmp1) {
            tmp1->parent_ = z;
        }
        y->right_ = z;
        z->parent_ = y;

        Node<Derived>* tmp2 = y->left_;
        x->right_ = tmp2;
        if (tmp2) {
            tmp2->parent_ = x;
        }
        y->left_ = x;
        x->parent_ = y;

        auto x_subtree_size = x->subtree_size;
        x->subtree_size -= z->subtree_size - (tmp2 ? tmp2->subtree_size : 0);
        z->subtree_size -= y->subtree_size - (tmp1 ? tmp1->subtree_size : 0);
        y->subtree_size = x_subtree_size;

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

    static Node<Derived>* rotate_left_right(Node<Derived>* x, Node<Derived>* z)
    {
        Node<Derived>* y = z->right_;
        Node<Derived>* tmp1 = y->left_;
        z->right_ = tmp1;
        if (tmp1) {
            tmp1->parent_ = z;
        }
        y->left_ = z;
        z->parent_ = y;

        Node<Derived>* tmp2 = y->right_;
        x->left_ = tmp2;
        if (tmp2) {
            tmp2->parent_ = x;
        }
        y->right_ = x;
        x->parent_ = y;

        auto x_subtree_size = x->subtree_size;
        x->subtree_size -= z->subtree_size - (tmp1 ? tmp1->subtree_size : 0);
        z->subtree_size -= y->subtree_size - (tmp1 ? tmp1->subtree_size : 0);
        y->subtree_size = x_subtree_size;

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

    Node<Derived>* begin_{nullptr};
    Node<Derived>* root_{nullptr};
};
