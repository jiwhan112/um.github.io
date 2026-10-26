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

// 비용만 검사하는 모델. 실제 register overlap/visibility/SDK 직렬화는 별도 검사다.
unsigned RootCost(unsigned constants, unsigned rootDescriptors, unsigned tables) {
    const std::uint64_t cost = static_cast < std::uint64_t > (constants)+2ull*rootDescriptors+tables;
    if (cost > 64) throw std::invalid_argument("64 DWORD budget exceeded");
    return static_cast < unsigned > (cost);
}
// root 슬롯과 HLSL 레지스터는 다른 번호 체계다. 비용 검사와 실제 SDK 검증은 별개다.
struct Binding { unsigned slot, shaderRegister, space;
};
unsigned FindSlot(const std::vector < Binding > & bindings, unsigned shaderRegister, unsigned space) {
    for (const auto& b:bindings) if (b.shaderRegister == shaderRegister && b.space == space) return b.slot;
    throw std::out_of_range("register not bound");
}
int main() {
    try {
        Check(RootCost(4, 0, 1) == 5, "renderer root size");
        Check(RootCost(1, 1, 0) == 3, "compute root size");
        Check(RootCost(64, 0, 0) == 64, "boundary");
        Reject([] { RootCost(65, 0, 0); }, "over budget");
        Reject([] { RootCost(63, 1, 0); }, "descriptor counts twice");
        Check(RootCost(0, 0, 0) == 0, "empty root");
        Reject([] { RootCost(UINT32_MAX, UINT32_MAX, UINT32_MAX); }, "no arithmetic wrap");
        const std::vector < Binding > b{{0, 3, 0}, {1, 0, 2}};
        Check(FindSlot(b, 3, 0) == 0, "slot differs from register");
        Check(FindSlot(b, 0, 2) == 1, "space distinct");
        Reject([&] { FindSlot(b, 0, 0); }, "wrong register space");
        std::cout << "PASS DX12-06: " << checks << " CPU contract checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
