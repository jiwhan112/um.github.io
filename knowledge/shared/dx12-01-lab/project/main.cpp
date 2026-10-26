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

// 어댑터 지원 결과를 받는 순수 선택 정책이다. D3D12CreateDevice를 호출하지 않는다.
struct Adapter { bool software;
bool createsAt11;
bool requiredFeature;
};
// 실제 DXGI 열거 대신, 이미 얻은 조사 결과만 선택한다.
int Choose(const std::vector < Adapter > & adapters, bool allowSoftware) {
    for (std::size_t i = 0;i < adapters.size();++i)
        if ((allowSoftware || !adapters[i].software) && adapters[i].createsAt11 && adapters[i].requiredFeature)
            return static_cast < int > (i);
    return -1;
}
int main() {
    try {
        Check(Choose({}, false) == -1, "empty");
        Check(Choose({{true, true, true}}, false) == -1, "no silent WARP");
        Check(Choose({{true, true, true}}, true) == 0, "explicit software");
        Check(Choose({{false, false, true}, {false, true, true}}, false) == 1, "probe failure");
        Check(Choose({{false, true, false}, {false, true, true}}, false) == 1, "feature separately checked");
        Check(Choose({{false, true, true}, {false, true, true}}, false) == 0, "stable first choice");
        Check(Choose({{true, true, true}, {false, true, true}}, false) == 1, "hardware only");
        Check(Choose({{false, true, false}}, true) == -1, "software flag cannot create feature");
        std::cout << "PASS DX12-01: " << checks << " CPU contract checks\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
