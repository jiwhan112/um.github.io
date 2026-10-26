// 트리의 자식은 소유하고 부모는 관찰만 한다. 강한 순환 소유를 만들지 않는다.
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
struct Node {
    inline static int alive = 0;
    int id;
    std::weak_ptr<Node> parent;
    std::vector<std::shared_ptr<Node>> children;
    explicit Node(int value):id(value) {
        ++alive;
    }
    ~Node() {
        --alive;
    }
};
int main() {
    unsigned checks = 0;
    auto check = [&](bool ok) {
        ++checks;
        if (!ok) throw std::runtime_error("ownership contract");
    };
    std::weak_ptr<Node> observer;
    {
        auto root = std::make_shared<Node>(1);
        auto child = std::make_shared<Node>(2);
        root->children.push_back(child);
        child->parent = root;
        observer = child;
        check(Node::alive == 2);
        auto locked = child->parent.lock();
        check(locked && locked->id == 1);
        // 잠시 lock한 부모 소유도 먼저 놓아야 마지막 root 해제로 정리된다.
        locked.reset();
        child.reset();
        check(!observer.expired());
        // root의 children이 아직 소유한다.
        root.reset();
        check(observer.expired() && Node::alive == 0);
        check(!observer.lock());
    }
    auto only = std::make_unique<int>(8);
    int* borrowed = only.get();
    check(*borrowed == 8);
    // 소유자가 살아 있고 reset 전인 이 구간에서만 사용.
    auto next = std::move(only);
    check(!only && *next == 8 && borrowed == next.get());
    next.reset();
    borrowed = nullptr;
    // 해제 뒤 역참조하지 않는다.
    check(borrowed == nullptr);
    std::cout << "tree-destroyed alive=0 weak-lock=empty\nPASS: " << checks << " contract checks\n";
}
