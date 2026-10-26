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
    if (!condition) throw std::runtime_error(label);
}

struct Item {
    int weight;
    int value;
};

int BestLoadout(const std::vector<Item>& items, int capacity) {
    if (items.size() > 20 || capacity < 0 || capacity > 100) {
        throw std::invalid_argument("bounds");
    }
    for (auto item : items) {
        if (item.weight < 1 || item.weight > 100 || item.value < 0 || item.value > 1000) {
            throw std::invalid_argument("item");
        }
    }
    std::vector<int> best(static_cast<std::size_t>(capacity) + 1, 0);
    for (auto item : items) {
        // 역순이므로 참조하는 상태는 아직 이번 아이템을 사용하지 않았습니다.
        for (int remaining = capacity; remaining >= item.weight; --remaining) {
            const auto current = static_cast<std::size_t>(remaining);
            const auto previous = static_cast<std::size_t>(remaining - item.weight);
            best[current] = std::max(best[current], best[previous] + item.value);
        }
    }
    return best.back();
}

// 테스트 오라클 전용: 호출자는 최대 9개 장비와 유효 범위를 제공합니다.
int Exhaustive(const std::vector<Item>& items, int capacity) {
    int answer = 0;
    const auto end = std::uint32_t{1} << items.size();
    for (std::uint32_t mask = 0; mask < end; ++mask) {
        int weight = 0;
        int value = 0;
        for (std::size_t index = 0; index < items.size(); ++index) {
            if (mask & (std::uint32_t{1} << index)) {
                weight += items[index].weight;
                value += items[index].value;
            }
        }
        if (weight <= capacity) {
            answer = std::max(answer, value);
        }
    }
    return answer;
}

int main() {
    Check(BestLoadout({}, 0) == 0, "empty capacity");
    Check(BestLoadout({{2, 3}}, 4) == 3, "single use not unbounded");
    Check(BestLoadout({{2, 3}, {3, 4}, {4, 5}}, 5) == 7, "combined optimum");
    std::mt19937 rng(1200);
    for (int trial = 0; trial < 300; ++trial) {
        std::vector<Item> items;
        for (int index = 0; index < trial % 10; ++index) {
            items.push_back({1 + static_cast<int>(rng() % 8), static_cast<int>(rng() % 15)});
        }
        for (int capacity = 0; capacity <= 15; ++capacity) {
            Check(BestLoadout(items, capacity) == Exhaustive(items, capacity), "subset oracle");
        }
    }
    bool rejected = false;
    try {
        BestLoadout({{0, 1}}, 5);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    Check(rejected, "zero weight policy");
    std::cout << "0/1 loadout optimum=7; 300 item sets against exhaustive subsets\n";
    std::cout << "PASS: " << checks << " contract checks\n";
}
