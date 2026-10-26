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
struct Event {
    int priority;
    int arrival;
};
void MergeSort(std::vector<Event> & a, std::size_t lo, std::size_t hi, std::vector<Event> & temp) {
    if (hi - lo < 2)
        return;
    const auto mid = lo +(hi - lo) / 2;
    MergeSort(a, lo, mid, temp);
    MergeSort(a, mid, hi, temp);
    std::size_t left = lo, right = mid, out = lo;
    while (left < mid && right < hi) {
        // 동점이면 왼쪽을 먼저 선택하여 입력 순서를 유지합니다.
        temp[out++] = a[right].priority < a[left].priority ? a[right++] : a[left++];
    }
    while (left < mid) temp[out++] = a[left++];
    while (right < hi) temp[out++] = a[right++];
    std::copy(temp.begin() + static_cast<std::ptrdiff_t>(lo), temp.begin() + static_cast<std::ptrdiff_t>(hi), a.begin() + static_cast<std::ptrdiff_t>(lo));
}
void Sort(std::vector<Event> & a) {
    std::vector<Event> temp(a.size());
    MergeSort(a, 0, a.size(), temp);
}
int main() {
    std::mt19937 rng(800);
    for (std::size_t n = 0; n < 128; ++n) {
        std::vector<Event> data;
        for (std::size_t i = 0; i < n; ++i) data.push_back({
            static_cast<int>(rng() % 7), static_cast<int>(i)
        });
        auto expected = data;
        std::stable_sort(expected.begin(), expected.end(), [](auto a, auto b) {
            return a.priority < b.priority;
        });
        Sort(data);
        Check(data.size() == expected.size(), "permutation size");
        for (std::size_t i = 0; i < n; ++i)
            Check(data[i].priority == expected[i].priority && data[i].arrival == expected[i].arrival, "stable oracle");
    }
    auto less =[](int a, int b) {
        return a < b;
    };
    for (int a = - 2; a <= 2; ++a)
        for (int b = - 2; b <= 2; ++b)
        for (int c = - 2; c <= 2; ++c) {
        Check(!less(a, a), "irreflexive");
        Check(!(less(a, b) && less(b, c)) || less(a, c), "transitive samples");
    }
    std::cout << "stable merge sort matches 128 duplicate-rich input lengths\nPASS: " << checks << " contract checks\n";
}
