#include "check.hpp"
#include <algorithm>
#include <deque>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
struct Item { int priority; int id; };
struct Less {
    bool operator()(const Item& a, const Item& b) const noexcept {
        return std::tie(a.priority, a.id) < std::tie(b.priority, b.id);
    }
};
int main() { return Run("containers", [](Checks& c) {
    std::vector<int> v{1,2,3,4}; const auto capacity = v.capacity();
    std::erase_if(v, [](int n) { return n % 2 == 0; });
    c.Expect(v == std::vector<int>({1,3}), "erase_if");
    c.Expect(v.capacity() == capacity, "erase retains capacity");
    v.insert(v.begin()+1, 2); c.Expect(v == std::vector<int>({1,2,3}), "middle insert");
    for (auto it = v.begin(); it != v.end();) { if (*it >= 2) it = v.erase(it); else ++it; }
    c.Expect(v == std::vector<int>({1}), "erase return iterator");
    std::list<int> nodes{3,1,2}; auto keep = std::next(nodes.begin());
    nodes.push_front(9); c.Expect(*keep == 1, "list iterator stable on insert");
    nodes.sort(); c.Expect(*keep == 1 && nodes.front() == 1, "list member sort");
    std::deque<int> queue; queue.push_front(2); queue.push_back(3); queue.push_front(1);
    c.Expect(queue[1] == 2, "deque random access");
    std::map<std::string, int, std::less<>> counts{{"potion",3}};
    c.Expect(counts.find("absent") == counts.end() && counts.size() == 1, "find no insert");
    c.Expect(counts["absent"] == 0 && counts.size() == 2, "brackets insert");
    const char a[] = "potion"; const char b[] = "potion";
    c.Expect(counts.at(a) == counts.at(b), "string content keys");
    std::unordered_map<int, std::string> lookup{{1,"one"}};
    const std::string* value = &lookup.at(1); lookup.rehash(100);
    c.Expect(value == &lookup.at(1) && *value == "one", "rehash preserves reference, not iterator");
    std::vector<Item> items{{2,3},{1,8},{2,1}}; std::ranges::sort(items, Less{});
    c.Expect(items[0].id == 8 && items[1].id == 1, "strict comparator");
    for (const auto& x : items) c.Expect(!Less{}(x,x), "irreflexivity sample");
    std::vector<std::unique_ptr<int>> owners; owners.push_back(std::make_unique<int>(7));
    owners.clear(); c.Expect(owners.empty(), "owning elements released");
}); }
