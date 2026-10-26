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

// descriptor도 GPU가 읽는 데이터이므로 retirement가 지나야 덮어쓴다.
struct Slot { bool occupied = false;
std::uint64_t retire = 0;
};
struct DescriptorPool {
    std::array < Slot, 2 > slots{};
    unsigned Allocate(std::uint64_t completed, std::uint64_t retire) {
        if (retire <= completed) throw std::invalid_argument("future retirement required");
        for (unsigned i = 0;i < slots.size();++i) {
            if (!slots[i].occupied || slots[i].retire <= completed) {
                slots[i] = {true, retire};
                return i;
            }
        }
        throw std::runtime_error("no safe slot");
    }
};
std::uint64_t Handle(std::uint64_t base, std::uint64_t stride, unsigned index, unsigned count) {
    if (index >= count || stride == 0 || (index && stride > (UINT64_MAX-base)/index))
        throw std::out_of_range("descriptor handle");
    return base+stride*index;
}
int main() {
    try {
        DescriptorPool p;
        Check(p.Allocate(0, 4) == 0, "first slot");
        Check(p.Allocate(0, 5) == 1, "second slot");
        Reject([&] { p.Allocate(3, 6); }, "inflight descriptors unavailable");
        Check(p.Allocate(4, 6) == 0, "retired slot reusable");
        Check(p.slots[1].retire == 5, "other descriptor unchanged");
        Reject([&] { p.Allocate(6, 6); }, "must retire in future");
        Check(Handle(1000, 32, 1, 2) == 1032, "device supplied stride");
        Reject([] { Handle(1000, 32, 2, 2); }, "index cap");
        Reject([] { Handle(UINT64_MAX-3, 8, 1, 2); }, "handle overflow");
        Reject([] { Handle(0, 0, 0, 2); }, "zero stride");
        std::cout << "PASS DX12-05: " << checks << " CPU contract checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
