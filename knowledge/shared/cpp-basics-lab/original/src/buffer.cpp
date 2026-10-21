#include "check.hpp"
#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>
// Educational int-only buffer. Not a replacement for std::vector.
class IntBuffer {
public:
    IntBuffer() = default;
    IntBuffer(const IntBuffer&) = delete;
    IntBuffer& operator=(const IntBuffer&) = delete;
    IntBuffer(IntBuffer&&) = delete;
    IntBuffer& operator=(IntBuffer&&) = delete;
    void PushBack(int value) {
        if (size_ == capacity_) {
            constexpr auto limit = std::numeric_limits<std::size_t>::max() / sizeof(int);
            if (capacity_ > limit / 2) throw std::length_error("capacity overflow");
            const auto next_capacity = capacity_ == 0 ? std::size_t{2} : capacity_ * 2;
            auto next = std::make_unique<int[]>(next_capacity);
            if (size_ != 0) std::copy_n(data_.get(), size_, next.get());
            data_ = std::move(next); // Old allocation released only after successful allocation/copy.
            capacity_ = next_capacity;
        }
        data_[size_++] = value;
    }
    int At(std::size_t index) const {
        if (index >= size_) throw std::out_of_range("index");
        return data_[index];
    }
    std::size_t Size() const { return size_; }
    std::size_t Capacity() const { return capacity_; }
private:
    std::unique_ptr<int[]> data_;
    std::size_t size_{};
    std::size_t capacity_{};
};
int main() {
    IntBuffer buffer; CHECK(buffer.Size() == 0);
    bool rejected = false; try { (void)buffer.At(0); } catch(const std::out_of_range&) { rejected = true; }
    CHECK(rejected);
    for (int i = 0; i < 100; ++i) buffer.PushBack(i);
    CHECK(buffer.Size() == 100); CHECK(buffer.Capacity() == 128); // This policy only.
    for (std::size_t i = 0; i < 100; ++i) CHECK(buffer.At(i) == static_cast<int>(i));
    Report();
}
