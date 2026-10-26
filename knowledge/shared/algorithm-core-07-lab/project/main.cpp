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
class MinHeap {
    std::vector<int> data_;
    public:
    void Push(int value) {
        data_.push_back(value);
        std::size_t child = data_.size() - 1;
        while (child > 0) {
            const auto parent =(child - 1) / 2;
            if (data_[parent] <= data_[child]) break;
            std::swap(data_[parent], data_[child]);
            child = parent;
        }
    }
    std::optional<int> Pop() {
        if (data_.empty())
            return std::nullopt;
        const int result = data_.front();
        data_.front() = data_.back();
        data_.pop_back();
        std::size_t parent = 0;
        // parent < n/2이면 왼쪽 자식이 존재합니다. 곱셈을 그 뒤에 수행합니다.
        while (parent < data_.size() / 2) {
            std::size_t child = parent * 2 + 1;
            if (child + 1 < data_.size() && data_[child + 1] < data_[child])++child;
            if (data_[parent] <= data_[child]) break;
            std::swap(data_[parent], data_[child]);
            parent = child;
        }
        return result;
    }
    bool Valid() const {
        for (std::size_t i = 1; i < data_.size(); ++i) if (data_[(i - 1) / 2] > data_[i])
            return false;
        return true;
    }
};
int main() {
    MinHeap heap;
    std::multiset<int > oracle;
    std::mt19937 rng(700);
    Check(!heap.Pop(), "empty pop");
    for (int i = 0; i < 2000; ++i) {
        if (rng() % 3 != 0) {
            int x = static_cast<int>(rng() % 101) - 50;
            heap.Push(x);
            oracle.insert(x);
        } else {
            auto x = heap.Pop();
            Check(x.has_value() == !oracle.empty(), "presence");
            if (!oracle.empty()) {
                Check(* x == * oracle.begin(), "minimum");
                oracle.erase(oracle.begin());
            }
        }
        Check(heap.Valid(), "parent child invariant");
    }
    while (!oracle.empty()) {
        Check(heap.Pop() == * oracle.begin(), "drain sorted");
        oracle.erase(oracle.begin());
    }
    Check(!heap.Pop(), "drained empty");
    std::cout << "min-heap tested with duplicates and mixed push/pop\nPASS: " << checks << " contract checks\n";
}
