#include <algorithm>
#include <array>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <cstring>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// GPU 호출이 아닌, 이 단원의 명시적 계약을 검증하는 결정적 CPU 모델이다.
static unsigned checks = 0;
void Check(bool condition, const char* name) {
    ++checks;
    if (!condition) throw std::runtime_error(name); // Release에서도 검사를 유지한다.
}
template < class F > void Reject(F action, const char* name) {
    bool rejected = false;
    try {
        action();
    } catch (const std::exception&) {
        rejected = true;
    }
    Check(rejected, name);
}

// 현재 backbuffer 번호와 각 슬롯의 마지막 GPU 참조 값을 함께 사용한다.
struct FramePool {
    std::array < std::uint64_t, 2 > retire{{0, 0}};
    bool CanUse(unsigned backBuffer, std::uint64_t completed) const {
        if (backBuffer >= retire.size()) throw std::out_of_range("swapchain index");
        return completed >= retire[backBuffer];
    }
    void Mark(unsigned index, std::uint64_t value) {
        if (index >= retire.size() || value == 0 || value <= retire[index])
            throw std::invalid_argument("bad retirement");
        retire[index] = value;
    }
};
int main() {
    try {
        FramePool p;
        Check(p.CanUse(0, 0), "initial0");
        Check(p.CanUse(1, 0), "initial1");
        p.Mark(0, 7);
        p.Mark(1, 8);
        Check(!p.CanUse(0, 6), "index returned is not completion");
        Check(p.CanUse(0, 7), "exact completion");
        Check(!p.CanUse(1, 7), "other slot pending");
        Check(p.CanUse(1, 8), "second complete");
        Reject([&] { p.CanUse(2, 100); }, "index bounds");
        Reject([&] { p.Mark(0, 0); }, "zero retire");
        Reject([&] { p.Mark(0, 6); }, "backward timeline");
        Check(p.retire[0] == 7, "rejected mark unchanged");
        std::cout << "PASS DX12-03: " << checks << " CPU contract checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
