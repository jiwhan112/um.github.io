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
std::size_t Lower(const std::vector<int> & a, int value) {
    std::size_t lo = 0, hi = a.size();
    // 답의 경계는 닫힌 범위 [lo, hi] 안에, 검사할 원소는 [lo, hi)에 있습니다.
    while (lo < hi) {
        const auto mid = lo +(hi - lo) / 2;
        if (a[mid] < value) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}
std::size_t Upper(const std::vector<int> & a, int value) {
    std::size_t lo = 0, hi = a.size();
    while (lo < hi) {
        auto mid = lo +(hi - lo) / 2;
        if (a[mid] <= value) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}
int main() {
    Check(Lower({}, 4) == 0, "empty end boundary");
    Check(Lower({4, 4, 4}, 4) == 0 && Upper({4, 4, 4}, 4) == 3, "duplicate interval");
    std::mt19937 rng(900);
    for (std::size_t n = 0; n < 128; ++n) {
        std::vector<int> a(n);
        for (auto & x : a) x = static_cast<int>(rng() % 21) - 10;
        std::sort(a.begin(), a.end());
        for (int x = - 12; x <= 12; ++x) {
            auto lo = Lower(a, x), hi = Upper(a, x);
            auto count = static_cast<std::size_t>(std::count(a.begin(), a.end(), x));
            Check(hi >= lo && hi - lo == count, "equal interval");
            Check(lo == static_cast<std::size_t>(std::find_if(a.begin(), a.end(), [x](int v) {
                return v >= x;
            }) - a.begin()), "linear boundary oracle");
        }
    }
    std::cout << "lower([4,4,4],4)=0 upper=3; insertion boundaries include size\nPASS: " << checks << " contract checks\n";
}
