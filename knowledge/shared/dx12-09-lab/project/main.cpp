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

// 이 모델의 두 고정 페이지는 일반 ring의 wrap/정렬까지 구현하지 않는다.
struct UploadPages {
    std::array < std::uint64_t, 2 > retire{{0, 0}};
    std::array < unsigned, 2 > used{{0, 0}};
    void Begin(unsigned slot, std::uint64_t completed) {
        if (slot >= 2 || completed < retire[slot]) throw std::logic_error("upload page in use");
        used[slot] = 0;
    }
    unsigned Allocate(unsigned slot, unsigned bytes) {
        if (slot >= 2 || bytes == 0 || bytes > 256-used[slot]) throw std::out_of_range("upload page capacity");
        unsigned offset = used[slot];
        used[slot]+= bytes;
        return slot*256+offset;
    }
    void Submit(unsigned slot, std::uint64_t value) {
        if (slot >= 2 || value <= retire[slot]) throw std::invalid_argument("retire timeline");
        retire[slot] = value;
    }
};
int main() {
    try {
        UploadPages p;
        p.Begin(0, 0);
        Check(p.Allocate(0, 64) == 0, "first span");
        Check(p.Allocate(0, 192) == 64, "remaining span");
        Reject([&] { p.Allocate(0, 1); }, "capacity");
        p.Submit(0, 3);
        Reject([&] { p.Begin(0, 2); }, "GPU still reads");
        Check(p.used[0] == 256, "failure keeps allocator");
        p.Begin(1, 2);
        Check(p.Allocate(1, 256) == 256, "second physical page");
        p.Submit(1, 4);
        p.Begin(0, 3);
        Check(p.Allocate(0, 128) == 0, "recycle completed page");
        Reject([&] { p.Allocate(0, 0); }, "zero allocation");
        Reject([&] { p.Begin(2, 99); }, "slot bounds");
        Reject([&] { p.Submit(0, 3); }, "monotonic retire");
        std::cout << "PASS DX12-09: " << checks << " CPU contract checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
