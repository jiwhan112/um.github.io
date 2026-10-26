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
#include <type_traits>
#include <vector>
// Release 빌드에서도 사라지지 않는 계약 검사입니다.
static std::size_t checks = 0;
void Check(bool condition, const char* label) {
    ++checks;
    if (!condition)
        throw std::runtime_error(label);
}
class RingQueue {
    static std::size_t ValidCapacity(std::size_t capacity) {
        // vector를 만들기 전에 입력 범위를 검사합니다.
        if (capacity == 0 || capacity > 1024) {
            throw std::invalid_argument("capacity");
        }
        return capacity;
    }
    std::vector<int> slots_;
    std::size_t head_ = 0, size_ = 0;
    public:
    // 저장소와 길이 상태를 분리해 이동하지 않도록 값 복사·이동을 금지합니다.
    RingQueue(const RingQueue&) = delete;
    RingQueue& operator=(const RingQueue&) = delete;
    RingQueue(RingQueue&&) = delete;
    RingQueue& operator=(RingQueue&&) = delete;
    explicit RingQueue(std::size_t capacity) : slots_(ValidCapacity(capacity)) {}
    bool Push(int value) {
        if (size_ == slots_.size())
            return false;
        slots_[(head_ + size_) % slots_.size()] = value;
        ++size_;
        return true;
    }
    std::optional<int> Pop() {
        if (size_ == 0)
            return std::nullopt;
        int value = slots_[head_];
        head_ =(head_ + 1) % slots_.size();
        --size_;
        return value;
    }
    std::size_t Size() const {
        return size_;
    }
};

// 회귀 검사: 복사·이동으로 불변식이 깨진 객체를 만들 수 없어야 합니다.
static_assert(!std::is_copy_constructible_v<RingQueue>, "explicit noncopyable/nonmovable policy");
static_assert(!std::is_copy_assignable_v<RingQueue>, "explicit noncopyable/nonmovable policy");
static_assert(!std::is_move_constructible_v<RingQueue>, "explicit noncopyable/nonmovable policy");
static_assert(!std::is_move_assignable_v<RingQueue>, "explicit noncopyable/nonmovable policy");

int main() {
    bool rejected = false;
    try {
        RingQueue invalid(0);
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    Check(rejected, "zero capacity");
    rejected = false;
    try {
        RingQueue invalid(std::numeric_limits<std::size_t>::max());
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    Check(rejected, "huge capacity rejected before allocation");
    RingQueue largest(1024);
    for (int value = 0; value < 1024; ++value) {
        Check(largest.Push(value), "maximum accepted capacity");
    }
    Check(!largest.Push(1024), "maximum capacity full");
    for (int value = 0; value < 1024; ++value) {
        Check(largest.Pop() == value, "maximum capacity FIFO");
    }
    Check(!largest.Pop(), "maximum capacity drained");
    for (std::size_t capacity : {1u, 2u, 7u}) {
        RingQueue q(capacity);
        std::deque<int> oracle;
        std::mt19937 rng(400);
        for (int i = 0; i < 1000; ++i) {
            if (rng() % 2) {
                bool room = oracle.size() < capacity;
                Check(q.Push(i) == room, "full rejection");
                if (room) oracle.push_back(i);
            } else {
                auto value = q.Pop();
                Check(value.has_value() == !oracle.empty(), "empty state");
                if (!oracle.empty()) {
                    Check(* value == oracle.front(), "FIFO");
                    oracle.pop_front();
                }
            }
            Check(q.Size() == oracle.size(), "ring size");
        }
    }
    std::vector<int> undo;
    for (int value : {10, 20, 30}) undo.push_back(value);
    for (int value : {30, 20, 10}) {
        Check(undo.back() == value, "LIFO");
        undo.pop_back();
    }
    std::cout << "FIFO ring capacities=1,2,7; LIFO undo order=30,20,10\nPASS: " << checks << " contract checks\n";
}
