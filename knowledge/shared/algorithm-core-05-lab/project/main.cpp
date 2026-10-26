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
class TinyMap {
    // 의도적으로 작은 버킷 수를 사용하여 충돌을 눈으로 확인합니다.
    std::array<std::vector<std::pair<std::uint32_t, int>>, 4> buckets_;
    public:
    void Put(std::uint32_t key, int value) {
        auto & bucket = buckets_[key % buckets_.size()];
        for (auto & entry : bucket) if (entry.first == key) {
            entry.second = value;
            return;
        }
        bucket.emplace_back(key, value);
    }
    std::optional<int> Get(std::uint32_t key) const {
        const auto & bucket = buckets_[key % buckets_.size()];
        for (const auto & entry : bucket) if (entry.first == key)
            return entry.second;
        return std::nullopt;
    }
    bool Erase(std::uint32_t key) {
        auto & bucket = buckets_[key % buckets_.size()];
        auto it = std::find_if(bucket.begin(), bucket.end(), [key](const auto & e) {
            return e.first == key;
        });
        if (it == bucket.end())
            return false;
        bucket.erase(it);
        return true;
    }
};
int main() {
    TinyMap map;
    std::map<std::uint32_t, int> oracle;
    map.Put(1, 10);
    map.Put(5, 50);
    map.Put(9, 90);
    Check(map.Get(1) == 10 && map.Get(5) == 50 && map.Get(9) == 90, "same bucket distinct keys");
    map.Put(5, 55);
    Check(map.Get(5) == 55, "replace not duplicate");
    Check(map.Erase(1) && map.Get(5) == 55 && !map.Get(1), "erase collision neighbor");
    TinyMap model;
    std::mt19937 rng(500);
    for (int i = 0; i < 1500; ++i) {
        auto key = rng() % 32;
        if (rng() % 3 == 0) {
            Check(model.Erase(key) ==(oracle.erase(key) != 0), "erase model");
        } else {
            model.Put(key, i);
            oracle[key] = i;
        }
        for (std::uint32_t k = 0; k < 32; ++k) {
            auto found = oracle.find(k);
            const auto actual = model.Get(k);
            Check(actual.has_value() ==(found != oracle.end()), "key existence");
            if (found != oracle.end())
                Check(* actual == found->second, "key value");
        }
    }
    std::cout << "collision keys=1,5,9; 1500 operations against ordered-map oracle\nPASS: " << checks << " contract checks\n";
}
