#include "check.hpp"
#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>

// int만 다루는 학습용 버퍼다. std::vector의 범용 구현이나 대체품이 아니다.
class IntBuffer {
public:
    IntBuffer() = default;
    // 복사·이동 정책을 구현하지 않았으므로 네 연산을 명시적으로 금지한다.
    IntBuffer(const IntBuffer&) = delete;
    IntBuffer& operator=(const IntBuffer&) = delete;
    IntBuffer(IntBuffer&&) = delete;
    IntBuffer& operator=(IntBuffer&&) = delete;

    void PushBack(int value) {
        if (size_ == capacity_) {
            // 원소 개수와 바이트 수의 범위를 생각하며, 두 배 계산 전에 상한을 검사한다.
            constexpr auto limit = std::numeric_limits<std::size_t>::max() / sizeof(int);
            if (capacity_ > limit / 2) throw std::length_error("capacity overflow");
            // 0 → 2 → 4 → 8 ... 은 이 클래스가 선택한 성장 정책이다.
            const auto next_capacity = capacity_ == 0 ? std::size_t{2} : capacity_ * 2;
            auto next = std::make_unique<int[]>(next_capacity);
            if (size_ != 0) std::copy_n(data_.get(), size_, next.get());
            // 새 할당과 int 복사를 마친 뒤 소유권을 옮긴다. 이때 이전 할당이 해제된다.
            // 일반 타입의 생성·복사 예외 처리까지 구현한 범용 컨테이너는 아니다.
            data_ = std::move(next);
            capacity_ = next_capacity;
        }
        data_[size_++] = value;
    }

    int At(std::size_t index) const {
        // 예약 공간이 아니라 현재 논리적 원소 수를 기준으로 접근을 제한한다.
        if (index >= size_) throw std::out_of_range("index");
        return data_[index];
    }

    std::size_t Size() const { return size_; }
    std::size_t Capacity() const { return capacity_; }

private:
    // unique_ptr가 할당을 소유하고 클래스 소멸 시 배열을 자동 해제한다.
    std::unique_ptr<int[]> data_;
    std::size_t size_{};
    std::size_t capacity_{};
};

int main() {
    IntBuffer buffer;
    CHECK(buffer.Size() == 0);
    bool rejected = false;
    try { (void)buffer.At(0); }
    catch(const std::out_of_range&) { rejected = true; }
    CHECK(rejected);

    for (int i = 0; i < 100; ++i) buffer.PushBack(i);
    CHECK(buffer.Size() == 100);
    // 128은 이 구현의 두 배 성장 결과다. std::vector의 capacity를 예측한 값이 아니다.
    CHECK(buffer.Capacity() == 128);
    // 이 한 CHECK 호출 지점이 원소마다 실행되어 런타임에는 100회를 센다.
    for (std::size_t i = 0; i < 100; ++i) CHECK(buffer.At(i) == static_cast<int>(i));
    Report();
}
