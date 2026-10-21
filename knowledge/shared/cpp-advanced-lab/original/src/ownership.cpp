#include "check.hpp"
#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>
struct FailPoint { inline static bool next_copy = false; };
// Teaching invariant: size == 0 iff data is null. Prefer vector in production.
class IntBuffer {
    std::size_t size_{};
    std::unique_ptr<int[]> data_;
    static std::unique_ptr<int[]> Copy(const IntBuffer& source) {
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
    IntBuffer(const IntBuffer& other) : size_(other.size_), data_(Copy(other)) {}
    IntBuffer& operator=(const IntBuffer& other) {
        if (this != &other) { IntBuffer temporary(other); Swap(temporary); }
        return *this;
    }
    IntBuffer(IntBuffer&& other) noexcept
        : size_(std::exchange(other.size_, 0)), data_(std::move(other.data_)) {}
    IntBuffer& operator=(IntBuffer&& other) noexcept {
        if (this != &other) { data_ = std::move(other.data_); size_ = std::exchange(other.size_, 0); }
        return *this;
    }
    void Swap(IntBuffer& other) noexcept { using std::swap; swap(size_, other.size_); data_.swap(other.data_); }
    friend void swap(IntBuffer& a, IntBuffer& b) noexcept { a.Swap(b); }
    std::size_t Size() const noexcept { return size_; }
    bool Valid() const noexcept { return (size_ == 0) == (data_ == nullptr); }
    int& At(std::size_t index) { if (index >= size_) throw std::out_of_range("index"); return data_[index]; }
    const int& At(std::size_t index) const { if (index >= size_) throw std::out_of_range("index"); return data_[index]; }
};
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
Immovable Make() { return Immovable{}; }
static_assert(std::is_nothrow_move_constructible_v<IntBuffer>);
static_assert(std::is_nothrow_move_assignable_v<IntBuffer>);
int main() { return Run("ownership", [](Checks& c) {
    RuleZero a{{1,2,3}}; auto b = a; b.values[0] = 9;
    c.Expect(a.values[0] == 1, "rule zero copies values");
    IntBuffer source({10,20,30}); IntBuffer copy(source); copy.At(0) = 77;
    c.Expect(source.At(0) == 10 && copy.At(0) == 77, "deep copy");
    IntBuffer target({5}); FailPoint::next_copy = true;
    c.Throws<std::bad_alloc>([&] { target = source; }, "injected copy failure");
    c.Expect(target.Size() == 1 && target.At(0) == 5 && target.Valid(), "strong guarantee");
    target = source; c.Expect(target.At(2) == 30, "copy assignment");
    IntBuffer& alias = target; target = alias; c.Expect(target.At(0) == 10, "self copy");
    IntBuffer moved(std::move(target));
    c.Expect(target.Size() == 0 && target.Valid(), "move source");
    c.Expect(moved.At(2) == 30 && moved.Valid(), "move destination");
    IntBuffer& moved_alias = moved; moved = std::move(moved_alias);
    c.Expect(moved.At(2) == 30 && moved.Valid(), "self move contract");
    IntBuffer empty; empty = std::move(moved);
    c.Expect(moved.Size() == 0 && empty.Size() == 3 && moved.Valid(), "move assignment");
    c.Throws<std::out_of_range>([&] { (void)moved.At(0); }, "moved source bounds");
    int copies{}; CopyOnly old(copies); CopyOnly copied(std::move(old)); (void)copied;
    c.Expect(copies == 1, "std::move can copy");
    auto elided = Make(); (void)elided;
    c.Expect(true, "prvalue construction despite deleted copy and move");
}); }
