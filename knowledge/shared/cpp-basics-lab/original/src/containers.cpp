#include "check.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
int Sum(std::span<const int> values) { int result{}; for (int value : values) result += value; return result; }
struct Item { int id; std::string name; int quantity; };
int main() {
    std::array<int, 3> a{10, 20, 30}; CHECK(Sum(a) == 60);
    std::vector<int> v; v.reserve(4);
    CHECK(v.empty()); CHECK(v.capacity() >= 4);
    v.resize(3); CHECK(v.size() == 3); CHECK(v[0] == 0);
    v[1] = 8; CHECK(Sum(v) == 8);
    bool rejected = false; try { (void)v.at(3); } catch (const std::out_of_range&) { rejected = true; }
    CHECK(rejected);
    const auto old_capacity = v.capacity();
    v.reserve(old_capacity + 1); // Do not retain or inspect any old element pointers.
    CHECK(v.capacity() > old_capacity); CHECK(v[1] == 8);
    int grid[2][3]{{1,2,3},{4,5,6}}; int (*row)[3] = grid;
    CHECK(row[1][2] == 6);
    std::vector<int> flat{1,2,3,4,5,6}; CHECK(flat.at(1 * 3 + 2) == 6);
    std::string embedded{"a\0b", 3};
    CHECK(embedded.size() == 3); CHECK(std::strlen(embedded.c_str()) == 1);
    std::string name{"Potion"}; const std::string_view view{name}; CHECK(view == "Potion");
    CHECK(std::strcmp("a", "b") < 0); CHECK(std::strcmp("b", "b") == 0);
    std::vector<Item> items{{2,"Mana",1},{1,"Health",3},{3,"Shield",1}};
    std::sort(items.begin(), items.end(), [](const Item& x, const Item& y) {
        return x.quantity != y.quantity ? x.quantity > y.quantity : x.id < y.id;
    });
    CHECK(items[0].id == 1); CHECK(items[1].id == 2); CHECK(items[2].id == 3);
    std::variant<int, std::string> data{7}; CHECK(std::get<int>(data) == 7);
    data = std::string{"text"}; CHECK(std::holds_alternative<std::string>(data));
    std::mt19937 engine_a{1234u}, engine_b{1234u};
    for (int i = 0; i < 8; ++i) CHECK(engine_a() == engine_b());
    std::uniform_int_distribution<int> die{1, 6};
    for (int i = 0; i < 20; ++i) { int roll = die(engine_a); CHECK(roll >= 1 && roll <= 6); }
    std::cout << "sizeof(Item)=" << sizeof(Item) << " alignof(Item)=" << alignof(Item) << '\n';
    Report();
}
