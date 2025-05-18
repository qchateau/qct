#include "avl.h"

class Data : public Node<Data> {
public:
    Data(int x) : x_(x) {}

    friend std::ostream& operator<<(std::ostream& os, Data const& data)
    {
        return os << data.x_;
    }

    friend auto operator<=>(Data const& lhs, Data const& rhs)
    {
        return lhs.x_ <=> rhs.x_;
    }

private:
    int x_;
};

int main()
{
    using Tree = AVLTree<Data>;
    auto tree = Tree{};
    auto nodes = std::list<Data>{};

    for (auto x : {200, 150, 250, 100, 110, 120, 50}) {
        nodes.push_back(Data{x});
        std::cout << "Inserting " << x << std::endl;
        tree.insert(nodes.back());
        std::cout << "===" << std::endl;
        std::cout << tree << std::endl;
        std::cout << "===" << std::endl;
    }

    return 0;
}
