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
class IdBuffer {
    std::unique_ptr<int[]> data_;
    std::size_t size_ = 0, capacity_ = 0, copied_ = 0;
    public:
    IdBuffer() = default;
    // 저장소와 길이 상태를 분리해 이동하지 않도록 값 복사·이동을 금지합니다.
    IdBuffer(const IdBuffer&) = delete;
    IdBuffer& operator=(const IdBuffer&) = delete;
    IdBuffer(IdBuffer&&) = delete;
    IdBuffer& operator=(IdBuffer&&) = delete;
    static constexpr std::size_t Limit = 1024;
    void Push(int id) {
        if (size_ == Limit)
            throw std::length_error("id limit");
        if (size_ == capacity_) {
            const auto next = capacity_ == 0 ? 1 : capacity_ * 2;
            auto replacement = std::make_unique<int[]>(next);
            // 새 할당이 성공한 뒤에만 기존 상태를 교체합니다.
            for (std::size_t i = 0; i < size_; ++i) replacement[i] = data_[i];
            copied_ += size_;
            data_ = std::move(replacement);
            capacity_ = next;
        }
        data_[size_++] = id;
    }
    int At(std::size_t i) const {
        if (i >= size_)
            throw std::out_of_range("id index");
        return data_[i];
    }
    std::size_t Size() const {
        return size_;
    }
    std::size_t Capacity() const {
        return capacity_;
    }
    std::size_t Copies() const {
        return copied_;
    }
};

// 회귀 검사: 복사·이동으로 불변식이 깨진 객체를 만들 수 없어야 합니다.
static_assert(!std::is_copy_constructible_v<IdBuffer>, "explicit noncopyable/nonmovable policy");
static_assert(!std::is_copy_assignable_v<IdBuffer>, "explicit noncopyable/nonmovable policy");
static_assert(!std::is_move_constructible_v<IdBuffer>, "explicit noncopyable/nonmovable policy");
static_assert(!std::is_move_assignable_v<IdBuffer>, "explicit noncopyable/nonmovable policy");

int main() {
    IdBuffer b;
    Check(b.Size() == 0 && b.Capacity() == 0, "empty buffer");
    for (std::size_t i = 0; i < IdBuffer::Limit; ++i) {
        b.Push(static_cast<int>(i));
        Check(b.At(i) == static_cast<int>(i), "appended identity");
        Check(b.Size() <= b.Capacity(), "capacity invariant");
        Check(b.Copies() < 2 * b.Size(), "geometric aggregate bound");
    }
    for (std::size_t i = 0; i < b.Size(); ++i)
        Check(b.At(i) == static_cast<int>(i), "preserved prefix");
    bool rejected = false;
    try {
        b.Push(9);
    } catch (const std::length_error &) {
        rejected = true;
    }
    Check(rejected && b.Size() == 1024 && b.At(1023) == 1023, "full leaves state");
    rejected = false;
    try {
        (void) b.At(1024);
    } catch (const std::out_of_range &) {
        rejected = true;
    }
    Check(rejected, "read bounds");
    std::cout << "1024 appends; growth copies=" << b.Copies() << "; capacity=" << b.Capacity() << '\n';
    std::cout << "PASS: " << checks << " contract checks\n";
}
