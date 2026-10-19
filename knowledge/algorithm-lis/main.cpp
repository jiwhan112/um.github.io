// AL03 Longest Increasing Subsequence: length and reconstructable witness.
// C++11-compliant; C++20 also tested. CPU-only reference model.
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace al03 {
// O(n log n) time, O(n) space. Strictly increasing (not nondecreasing).
std::vector<std::size_t> lis_indices(const std::vector<int>& a) {
    const std::size_t n = a.size();
    std::vector<int> tails;
    std::vector<std::size_t> last;
    std::vector<std::size_t> parent(n, n);  // n is our sentinel.
    tails.reserve(n);
    last.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        const std::size_t level = static_cast<std::size_t>(
            std::lower_bound(tails.begin(), tails.end(), a[i]) - tails.begin());
        if (level > 0) parent[i] = last[level - 1];
        if (level == tails.size()) {
            tails.push_back(a[i]);
            last.push_back(i);
        } else {
            tails[level] = a[i];
            last[level] = i;
        }
    }
    std::vector<std::size_t> result;
    if (last.empty()) return result;
    for (std::size_t i = last.back(); i != n; i = parent[i]) result.push_back(i);
    std::reverse(result.begin(), result.end());
    return result;
}

// Independent O(n^2) oracle used only for test, not the optimized solution.
std::size_t quadratic_lis_length(const std::vector<int>& a) {
    if (a.empty()) return 0;
    std::vector<std::size_t> dp(a.size(), 1);
    std::size_t best = 1;
    for (std::size_t i = 0; i < a.size(); ++i) {
        for (std::size_t j = 0; j < i; ++j) {
            if (a[j] < a[i]) dp[i] = std::max(dp[i], dp[j] + 1);
        }
        best = std::max(best, dp[i]);
    }
    return best;
}
}

struct Tester {
    std::size_t checks = 0;
    void require(bool ok, const char* message) {
        ++checks;
        if (!ok) throw std::runtime_error(message);
    }
};

int main() {
    try {
        Tester t;
        const std::vector<std::vector<int> > fixed = {
            {}, {3}, {1,1,1,1}, {1,2,3,4}, {4,3,2,1},
            {10,9,2,5,3,7,101,18}, {-9,-9,-2,-7,3}, {2,2,3,1,4,4},
            {5,1,4,2,3}, {-3,-2,-1,-1,0}, {0,-1,0,-1,0}, {7,7,7}
        };
        std::mt19937 rng(20261017);
        std::uniform_int_distribution<int> len(0, 75);
        std::uniform_int_distribution<int> value(-40, 40);
        for (std::size_t round = 0; round < fixed.size() + 1600; ++round) {
            std::vector<int> a;
            if (round < fixed.size()) a = fixed[round];
            else {
                const int n = len(rng);
                for (int i = 0; i < n; ++i) a.push_back(value(rng));
            }
            const std::vector<std::size_t> indices = al03::lis_indices(a);
            t.require(indices.size() == al03::quadratic_lis_length(a), "LIS length mismatch");
            for (std::size_t j = 0; j < indices.size(); ++j) {
                t.require(indices[j] < a.size(), "LIS index range");
                if (j != 0) {
                    t.require(indices[j-1] < indices[j], "LIS original order");
                    t.require(a[indices[j-1]] < a[indices[j]], "LIS strict order");
                }
            }
        }
        const std::vector<int> nums = {10, 9, 2, 5, 3, 7, 101, 18};
        t.require(al03::lis_indices(nums).size() == 4, "classic LIS length");
        t.require(al03::lis_indices(std::vector<int>{5,5,5}).size() == 1,
                  "duplicates should NOT extend strict LIS");
        std::cout << "PASS AL03 LIS; checks=" << t.checks << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL AL03 " << e.what() << '\n';
        return 1;
    }
}
