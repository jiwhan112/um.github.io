#include <algorithm>
#include <array>
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <type_traits>
#include <vector>
// Release 빌드에서도 사라지지 않는 계약 검사입니다.
static std::size_t checks = 0;
void Check(bool condition, const char* label) {
    ++checks;
    if (!condition)
        throw std::runtime_error(label);
}
class SearchTree {
    struct Node {
        int key;
        std::unique_ptr<Node> left, right;
        explicit Node(int k) : key(k) {}
    };
    std::unique_ptr<Node> root_;
    std::size_t size_ = 0;
    public:
    SearchTree() = default;
    // 저장소와 길이 상태를 분리해 이동하지 않도록 값 복사·이동을 금지합니다.
    SearchTree(const SearchTree&) = delete;
    SearchTree& operator=(const SearchTree&) = delete;
    SearchTree(SearchTree&&) = delete;
    SearchTree& operator=(SearchTree&&) = delete;
    bool Insert(int key) {
        auto* link = & root_;
        while (* link) {
            if ((* link)->key == key)
                return false;
            link = key <(* link)->key ? &(* link)->left : &(* link)->right;
        }
        // 이 교육용 트리는 균형화하지 않으므로 깊이를 제한합니다.
        if (size_ == 128)
            throw std::length_error("tree limit");
        * link = std::make_unique<Node>(key);
        ++size_;
        return true;
    }
    bool Contains(int key) const {
        const Node* node = root_.get();
        while (node) {
            if (node->key == key)
                return true;
            node = key < node->key ? node->left.get() : node->right.get();
        }
        return false;
    }
    std::vector<int> Ordered() const {
        std::vector<int> out;
        std::vector<const Node *> stack;
        const Node* node = root_.get();
        while (node || !stack.empty()) {
            while (node) {
                stack.push_back(node);
                node = node->left.get();
            }
            node = stack.back();
            stack.pop_back();
            out.push_back(node->key);
            node = node->right.get();
        }
        return out;
    }
};

// 회귀 검사: 복사·이동으로 불변식이 깨진 객체를 만들 수 없어야 합니다.
static_assert(!std::is_copy_constructible_v<SearchTree>, "explicit noncopyable/nonmovable policy");
static_assert(!std::is_copy_assignable_v<SearchTree>, "explicit noncopyable/nonmovable policy");
static_assert(!std::is_move_constructible_v<SearchTree>, "explicit noncopyable/nonmovable policy");
static_assert(!std::is_move_assignable_v<SearchTree>, "explicit noncopyable/nonmovable policy");

int main() {
    SearchTree tree;
    std::set<int> oracle;
    Check(!tree.Contains(1) && tree.Ordered().empty(), "empty tree");
    std::mt19937 rng(600);
    for (int i = 0; i < 500; ++i) {
        int key = static_cast<int>(rng() % 100) - 50;
        Check(tree.Insert(key) == oracle.insert(key).second, "duplicate set policy");
        Check(tree.Ordered() == std::vector<int>(oracle.begin(), oracle.end()), "inorder order");
    }
    for (int k = - 60; k <= 60; ++k)
        Check(tree.Contains(k) ==(oracle.count(k) != 0), "lookup");
    SearchTree chain;
    for (int k = 0; k < 128; ++k)
        Check(chain.Insert(k), "ordered worst shape");
    bool rejected = false;
    try {
        chain.Insert(128);
    } catch (const std::length_error &) {
        rejected = true;
    }
    Check(rejected && chain.Contains(127), "bounded depth");
    std::cout << "BST order verified; ascending input creates an unbalanced chain\nPASS: " << checks << " contract checks\n";
}
