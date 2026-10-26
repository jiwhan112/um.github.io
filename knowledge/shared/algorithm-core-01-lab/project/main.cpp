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
struct ScanResult {
    std::uint64_t sum;
    std::size_t visits;
};
ScanResult TotalDamage(const std::vector<std::uint32_t> & damage) {
    if (damage.size() > 100000)
        throw std::length_error("too many events");
    std::uint64_t sum = 0;
    std::size_t visits = 0;
    // 불변식: 처리한 접두사의 합과 visits가 같은 경계까지 진행합니다.
    for (auto value : damage) {
        sum += value;
        ++visits;
    }
    return {sum, visits};
}
std::uint64_t PairChecks(std::size_t count) {
    if (count > 2000)
        throw std::length_error("pair budget");
    std::uint64_t visits = 0;
    for (std::size_t i = 0; i < count; ++i)
        for (std::size_t j = i + 1; j < count; ++j)++visits;
    return visits;
}
int main() {
    Check(TotalDamage({}).sum == 0, "empty identity");
    Check(TotalDamage({7, 0, 9}).sum == 16, "damage sum");
    Check(TotalDamage({UINT32_MAX, UINT32_MAX}).sum == 8589934590ULL, "wide sum");
    for (std::size_t n = 0; n <= 128; ++n) {
        const auto r = TotalDamage(std::vector<std::uint32_t>(n, 3));
        Check(r.sum == 3 * n && r.visits == n, "linear trace");
        const auto expected = n == 0 ? 0 : n *(n - 1) / 2;
        Check(PairChecks(n) == expected, "triangular count");
    }
    bool rejected = false;
    try {
        PairChecks(2001);
    } catch (const std::length_error &) {
        rejected = true;
    }
    Check(rejected, "bounded pair work");
    std::cout << "n=8: scan=8 pair=28; n=16: scan=16 pair=120\n";
    std::cout << "PASS: " << checks << " contract checks\n";
}
