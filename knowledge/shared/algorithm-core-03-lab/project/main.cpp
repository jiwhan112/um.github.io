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
#include <vector>
// Release 빌드에서도 사라지지 않는 계약 검사입니다.
static std::size_t checks = 0;
void Check(bool condition, const char* label) {
    ++checks;
    if (!condition)
        throw std::runtime_error(label);
}
class EventList {
    struct Node {
        int id;
        std::unique_ptr<Node> next;
        explicit Node(int v) : id(v) {}
    };
    std::unique_ptr<Node> head_;
    std::size_t size_ = 0;
    public:
    EventList() = default;
    EventList(const EventList &) = delete;
    EventList & operator =(const EventList &) = delete;
    ~ EventList() {
        Clear();
    }
    void PushFront(int id) {
        auto node = std::make_unique<Node>(id);
        node->next = std::move(head_);
        head_ = std::move(node);
        ++size_;
    }
    bool EraseFirst(int id) {
        auto* link = & head_;
        while (* link &&(* link)->id != id) link = &((* link)->next);
        if (! * link)
            return false;
        // 지울 노드의 소유권을 분리한 후 다음 노드를 연결합니다.
        auto removed = std::move(* link);
        * link = std::move(removed->next);
        --size_;
        return true;
    }
    void Clear() noexcept {
        // 긴 사슬의 재귀 소멸을 피합니다.
        while (head_) {
            auto node = std::move(head_);
            head_ = std::move(node->next);
        }
        size_ = 0;
    }
    std::vector<int> Values() const {
        std::vector<int> values;
        for (const Node* p = head_.get(); p; p = p->next.get()) values.push_back(p->id);
        return values;
    }
    std::size_t Size() const {
        return size_;
    }
};
int main() {
    EventList list;
    std::vector<int> oracle;
    Check(!list.EraseFirst(4), "empty erase");
    for (int id : {4, 2, 4, 9}) {
        list.PushFront(id);
        oracle.insert(oracle.begin(), id);
    }
    for (int id : {4, 9, 4, 77, 2}) {
        auto it = std::find(oracle.begin(), oracle.end(), id);
        bool exists = it != oracle.end();
        if (exists) oracle.erase(it);
        Check(list.EraseFirst(id) == exists, "erase existence");
        Check(list.Values() == oracle && list.Size() == oracle.size(), "ownership chain");
    }
    std::mt19937 rng(301);
    for (int i = 0; i < 1000; ++i) {
        int id = static_cast<int>(rng() % 20);
        if (rng() % 2) {
            list.PushFront(id);
            oracle.insert(oracle.begin(), id);
        } else {
            auto it = std::find(oracle.begin(), oracle.end(), id);
            bool exists = it != oracle.end();
            if (exists) oracle.erase(it);
            Check(list.EraseFirst(id) == exists, "random erase");
        }
        Check(list.Values() == oracle && list.Size() == oracle.size(), "random sequence");
    }
    list.Clear();
    Check(list.Size() == 0 && list.Values().empty(), "clear");
    std::cout << "head/middle/tail/duplicate erase and 1000 model operations\nPASS: " << checks << " contract checks\n";
}
