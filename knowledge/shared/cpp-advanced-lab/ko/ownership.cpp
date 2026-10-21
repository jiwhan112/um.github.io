#include "check.hpp"
#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>

// 실제 메모리를 고갈시키지 않고 다음 Copy 호출의 실패 경로를 선택한다.
struct FailPoint { inline static bool next_copy = false; };

// 불변식: size가 0일 때에만 data가 null이다. int 전용 학습 구현이다.
// 실제 프로젝트에서는 먼저 std::vector 같은 소유 타입을 선택한다.
class IntBuffer {
    std::size_t size_{};
    std::unique_ptr<int[]> data_;

    static std::unique_ptr<int[]> Copy(const IntBuffer& source) {
        // 플래그를 소비한 뒤 예외를 던진다. 대상 버퍼의 보장과 이 실험 플래그는 별개다.
        if (std::exchange(FailPoint::next_copy, false)) throw std::bad_alloc{};
        if (source.size_ == 0) return {};
        auto result = std::make_unique<int[]>(source.size_);
        std::copy_n(source.data_.get(), source.size_, result.get());
        return result;
    }

public:
    IntBuffer() = default;
    explicit IntBuffer(std::initializer_list<int> values)
        : size_(values.size()), data_(size_ ? std::make_unique<int[]>(size_) : nullptr) {
        if (size_ != 0) std::copy(values.begin(), values.end(), data_.get());
    }
    ~IntBuffer() = default;

    // 서로 다른 배열을 소유하도록 깊은 복사한다.
    IntBuffer(const IntBuffer& other) : size_(other.size_), data_(Copy(other)) {}
    IntBuffer& operator=(const IntBuffer& other) {
        if (this != &other) {
            // 준비가 실패하면 이 객체는 그대로다. 성공한 임시값과만 상태를 교체한다.
            IntBuffer temporary(other);
            Swap(temporary);
        }
        return *this;
    }

    IntBuffer(IntBuffer&& other) noexcept
        : size_(std::exchange(other.size_, 0)), data_(std::move(other.data_)) {}
    IntBuffer& operator=(IntBuffer&& other) noexcept {
        // 이 타입은 자기 이동에서도 기존 값을 유지하도록 직접 계약을 정했다.
        if (this != &other) {
            data_ = std::move(other.data_);
            size_ = std::exchange(other.size_, 0);
        }
        return *this;
    }

    void Swap(IntBuffer& other) noexcept {
        using std::swap;
        swap(size_, other.size_);
        data_.swap(other.data_);
    }
    friend void swap(IntBuffer& a, IntBuffer& b) noexcept { a.Swap(b); }
    std::size_t Size() const noexcept { return size_; }
    bool Valid() const noexcept { return (size_ == 0) == (data_ == nullptr); }
    int& At(std::size_t index) {
        if (index >= size_) throw std::out_of_range("index");
        return data_[index];
    }
    const int& At(std::size_t index) const {
        if (index >= size_) throw std::out_of_range("index");
        return data_[index];
    }
};

// vector가 소유권을 관리하므로 직접 복사·이동·소멸 코드를 작성하지 않는다.
struct RuleZero { std::vector<int> values; };

struct CopyOnly {
    int* copies;
    explicit CopyOnly(int& value) : copies(&value) {}
    CopyOnly(const CopyOnly& other) : copies(other.copies) { ++*copies; }
};

struct Immovable {
    Immovable() = default;
    Immovable(const Immovable&) = delete;
    Immovable(Immovable&&) = delete;
};

// C++17 이후 같은 타입 prvalue로 결과 객체를 직접 만든다. 이름 붙은 지역값 NRVO와 다르다.
Immovable Make() { return Immovable{}; }
static_assert(std::is_nothrow_move_constructible_v<IntBuffer>);
static_assert(std::is_nothrow_move_assignable_v<IntBuffer>);

int main() {
    return Run("ownership", [](Checks& c) {
        RuleZero a{{1,2,3}};
        auto b = a;
        b.values[0] = 9;
        c.Expect(a.values[0] == 1, "rule zero copies values");

        IntBuffer source({10,20,30});
        IntBuffer copy(source);
        copy.At(0) = 77;
        c.Expect(source.At(0) == 10 && copy.At(0) == 77, "deep copy");

        IntBuffer target({5});
        FailPoint::next_copy = true;
        c.Throws<std::bad_alloc>([&] { target = source; }, "injected copy failure");
        c.Expect(target.Size() == 1 && target.At(0) == 5 && target.Valid(), "strong guarantee");
        target = source;
        c.Expect(target.At(2) == 30, "copy assignment");
        IntBuffer& alias = target;
        target = alias;
        c.Expect(target.At(0) == 10, "self copy");

        IntBuffer moved(std::move(target));
        // 비어 있는 이동 원본은 이 IntBuffer의 명시적 계약이며 모든 타입의 공통 결과가 아니다.
        c.Expect(target.Size() == 0 && target.Valid(), "move source");
        c.Expect(moved.At(2) == 30 && moved.Valid(), "move destination");
        IntBuffer& moved_alias = moved;
        moved = std::move(moved_alias);
        c.Expect(moved.At(2) == 30 && moved.Valid(), "self move contract");
        IntBuffer empty;
        empty = std::move(moved);
        c.Expect(moved.Size() == 0 && empty.Size() == 3 && moved.Valid(), "move assignment");
        c.Throws<std::out_of_range>([&] { (void)moved.At(0); }, "moved source bounds");

        int copies{};
        CopyOnly old(copies);
        // std::move는 이동 생성자 호출을 강제하지 않는다. 여기서는 const& 복사 생성자가 선택된다.
        CopyOnly copied(std::move(old));
        (void)copied;
        c.Expect(copies == 1, "std::move can copy");
        auto elided = Make();
        (void)elided;
        // 삭제된 복사·이동으로도 위 선언이 컴파일되는 사실이 핵심 증거다.
        // 이 Expect(true)는 원본 실행 카운터에 포함되며 추가 동작을 검증하지 않는다.
        c.Expect(true, "prvalue construction despite deleted copy and move");
    });
}
